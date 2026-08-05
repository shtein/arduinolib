import argparse
import socket
import time


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


# DDP protocol constants
DDP_VER1 = 0x40           # version 1 in bits 6-7 of the flags byte
DDP_FLAG_PUSH = 0x01      # "display this frame now"
DDP_DATA_RGB = 0x01       # data type: RGB, 8 bits/channel (receiver ignores it)
DDP_DEST_ID = 0x01        # output id 1 = default (receiver ignores it)
DDP_MAX_DATA = 1440       # max payload per packet (480 RGB pixels)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Test a DDP-compatible LED controller over UDP."
    )

    parser.add_argument(
        "--host",
        required=True,
        help="Controller IP address, for example 192.168.1.50",
    )

    parser.add_argument(
        "--port",
        type=int,
        default=4048,
        help="DDP UDP port. Default: 4048",
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


def build_packets(
    led_count: int,
    red: int,
    green: int,
    blue: int,
    sequence: int,
) -> list[bytes]:
    """Build the DDP packet(s) for one solid-color frame.

    A frame larger than DDP_MAX_DATA bytes is split across packets: each carries
    its own byte offset, and only the final packet sets the PUSH flag so the
    receiver displays a fully assembled frame.
    """
    if led_count < 1:
        raise ValueError("LED count must be at least 1.")

    pixels = bytes([red, green, blue]) * led_count
    total = len(pixels)

    packets: list[bytes] = []
    offset = 0
    while offset < total:
        chunk = pixels[offset:offset + DDP_MAX_DATA]
        is_last = (offset + len(chunk)) >= total

        flags = DDP_VER1 | (DDP_FLAG_PUSH if is_last else 0x00)
        length = len(chunk)

        header = bytes(
            [
                flags,
                sequence & 0x0F,
                DDP_DATA_RGB,
                DDP_DEST_ID,
                (offset >> 24) & 0xFF,
                (offset >> 16) & 0xFF,
                (offset >> 8) & 0xFF,
                offset & 0xFF,
                (length >> 8) & 0xFF,
                length & 0xFF,
            ]
        )

        packets.append(header + chunk)
        offset += len(chunk)

    return packets


def send_frame(
    sock: socket.socket,
    host: str,
    port: int,
    packets: list[bytes],
) -> None:
    for packet in packets:
        sock.sendto(packet, (host, port))


def main() -> None:
    args = parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        print(
            f"Streaming DDP to {args.host}:{args.port}, "
            f"LEDs={args.leds}"
        )

        # DDP sequence numbers run 1-15 (0 = unused); wrap around.
        sequence = 1

        while True:
            for name, red, green, blue in COLORS:
                packets = build_packets(
                    args.leds,
                    red,
                    green,
                    blue,
                    sequence,
                )

                send_frame(sock, args.host, args.port, packets)

                print(
                    f"TX: {name} "
                    f"RGB=({red}, {green}, {blue}), "
                    f"LEDs={args.leds}, "
                    f"packets={len(packets)}, seq={sequence}"
                )

                sequence = sequence + 1 if sequence < 15 else 1
                time.sleep(args.interval)

    except KeyboardInterrupt:
        print("\nStopping")

        send_frame(
            sock,
            args.host,
            args.port,
            build_packets(args.leds, 0, 0, 0, sequence),
        )

    finally:
        sock.close()


if __name__ == "__main__":
    main()
