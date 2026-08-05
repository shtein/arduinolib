import argparse
import threading
import time

import serial


COLORS = [
    ("Red", 255, 0, 0),
    ("Green", 0, 255, 0),
    ("Blue", 0, 0, 255),
    ("White", 255, 255, 255),
    ("Yellow", 255, 255, 0),
    ("Cyan", 0, 255, 255),
    ("Magenta", 255, 0, 255),
    ("Off", 0, 0, 0),
]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Test an Adalight-compatible LED controller."
    )

    parser.add_argument(
        "--port",
        required=True,
        help="Serial port, for example /dev/cu.usbserial-XXXX",
    )

    parser.add_argument(
        "--baud",
        type=int,
        default=115200,
        help="Serial baud rate. Default: 115200",
    )

    parser.add_argument(
        "--leds",
        type=int,
        default=25,
        help="Number of LEDs. Default: 25",
    )

    parser.add_argument(
        "--interval",
        type=float,
        default=1.0,
        help="Seconds between colors. Default: 1.0",
    )

    return parser.parse_args()


def build_frame(
    led_count: int,
    red: int,
    green: int,
    blue: int,
) -> bytes:
    if not 1 <= led_count <= 65536:
        raise ValueError("LED count must be between 1 and 65536.")

    encoded_count = led_count - 1
    high = (encoded_count >> 8) & 0xFF
    low = encoded_count & 0xFF
    checksum = high ^ low ^ 0x55

    header = bytes(
        [
            ord("A"),
            ord("d"),
            ord("a"),
            high,
            low,
            checksum,
        ]
    )

    pixels = bytes([red, green, blue]) * led_count
    return header + pixels


def read_serial(ser: serial.Serial) -> None:
    while ser.is_open:
        try:
            data = data = ser.readline()

            if data:
                print(f"RX raw: {data!r}")

                text = data.decode("utf-8", errors="replace")
                print(f"RX text: {text}", end="")

        except serial.SerialException as error:
            print(f"\nSerial read error: {error}")
            return


def main() -> None:
    args = parse_args()

    with serial.Serial(
        port=args.port,
        baudrate=args.baud,
        timeout=0.1,
        write_timeout=1,
    ) as ser:
        print(
            f"Connected to {args.port} at {args.baud} baud, "
            f"LEDs={args.leds}"
        )

        reader = threading.Thread(
            target=read_serial,
            args=(ser,),
            daemon=True,
        )
        reader.start()

        try:
            while True:
                for name, red, green, blue in COLORS:
                    frame = build_frame(
                        args.leds,
                        red,
                        green,
                        blue,
                    )

                    ser.write(frame)
                    ser.flush()

                    print(
                        f"TX: {name} "
                        f"RGB=({red}, {green}, {blue}), "
                        f"LEDs={args.leds}"
                    )

                    time.sleep(args.interval)

        except KeyboardInterrupt:
            print("\nStopping")

            ser.write(
                build_frame(
                    args.leds,
                    0,
                    0,
                    0,
                )
            )
            ser.flush()


if __name__ == "__main__":
    main()

