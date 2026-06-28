import serial
import datetime
import sys

PORT     = "COM9"
BAUDRATE = 57600
LOG_FILE = r"D:\Kalantar Zadeh\LABKHAND50.261 - Copy\LABKHAND50.261_403.1\log.txt"

def main():
    print(f"Opening {PORT} at {BAUDRATE} baud...")
    print(f"Logging to: {LOG_FILE}")
    print("Press Ctrl+C to stop.\n")

    try:
        ser = serial.Serial(PORT, BAUDRATE, timeout=1)
    except serial.SerialException as e:
        print(f"[ERROR] Could not open {PORT}: {e}")
        sys.exit(1)

    line_buffer = b""

    with open(LOG_FILE, "a", encoding="utf-8", errors="replace") as f:
        session_start = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        header = f"\n{'='*60}\n  SESSION STARTED: {session_start}\n{'='*60}\n"
        f.write(header)
        print(header)

        try:
            while True:
                byte = ser.read(1)
                if not byte:
                    continue

                line_buffer += byte

                if byte in (b'\n', b'\r'):
                    if line_buffer.strip():
                        ts   = datetime.datetime.now().strftime("%H:%M:%S.%f")[:-3]
                        line = line_buffer.decode("utf-8", errors="replace").strip()
                        entry = f"[{ts}]  {line}\n"
                        f.write(entry)
                        f.flush()
                        print(entry, end="")
                    line_buffer = b""

        except KeyboardInterrupt:
            footer = f"\n{'='*60}\n  SESSION ENDED: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n{'='*60}\n"
            f.write(footer)
            print(footer)

        finally:
            ser.close()
            print("Serial port closed.")

if __name__ == "__main__":
    main()
