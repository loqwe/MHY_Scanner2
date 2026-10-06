"""Exercise FFmpeg against an HTTP server that never sends its response body."""

import socket
import subprocess
import sys
import threading


def verify(executable, mode):
    done = threading.Event()
    errors = []
    with socket.socket() as listener:
        listener.bind(("127.0.0.1", 0))
        listener.listen(1)
        listener.settimeout(4)
        port = listener.getsockname()[1]

        def stall():
            try:
                connection, _ = listener.accept()
                with connection:
                    connection.settimeout(2)
                    connection.recv(8192)
                    connection.sendall(
                        b"HTTP/1.1 200 OK\r\nContent-Type: video/x-flv\r\n"
                        b"Content-Length: 100000\r\nConnection: close\r\n\r\n"
                    )
                    done.wait(4)
            except Exception as error:
                errors.append(error)

        worker = threading.Thread(target=stall)
        worker.start()
        try:
            subprocess.run(
                [executable, f"http://127.0.0.1:{port}/stall.flv", mode],
                check=True,
                timeout=3,
            )
        finally:
            done.set()
            worker.join()
        if errors:
            raise errors[0]


if __name__ == "__main__":
    verify(sys.argv[1], "timeout")
    verify(sys.argv[1], "cancel")
