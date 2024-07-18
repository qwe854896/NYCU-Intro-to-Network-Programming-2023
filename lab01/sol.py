#!/usr/bin/env python3

import multiprocessing
import subprocess
import sys
import time


def run_command(command):
    print(f"Running command: {command}")
    subprocess.run(command, shell=True)


if __name__ == "__main__":
    uid = sys.argv[1]

    run_command(f"rm -rf tcpdump.pcap")

    command = "sudo tcpdump -ni any -Xxnv udp and port 10495 -w tcpdump.pcap"
    process = multiprocessing.Process(target=run_command, args=(command,))
    process.start()

    time.sleep(3)

    print("Sending hello...")
    run_command(f"python3 cap.py {uid}")

    time.sleep(5)

    print("Decoding...")
    run_command(f"python3 decode.py tcpdump.pcap")

    process.terminate()
