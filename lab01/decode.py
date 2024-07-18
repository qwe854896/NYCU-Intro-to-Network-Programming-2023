from scapy.all import *
import sys


def calculate_x_header_length(packet):
    x_header_length = 0

    # Calculate the length of IP options
    if IP in packet:
        ip_options_length = len(packet[IP].options)
        x_header_length += ip_options_length
    else:
        return 0

    # Calculate the length of the UDP header and payload
    if UDP in packet:
        udp_header_length = len(packet[UDP])
        udp_payload_length = len(packet[UDP].payload)
        x_header_length += udp_payload_length
    else:
        return 0

    return x_header_length


def extract_udp_payload(packet):
    if UDP in packet:
        udp_payload = packet[UDP].payload
        if udp_payload:
            return bytes(udp_payload).decode("utf-8", errors="ignore")
    return ""


def extract_flag(pcap_file_path):
    flag = ""
    packets = [0] * 256

    scapy_cap = rdpcap(pcap_file_path)

    for packet in scapy_cap:
        if UDP in packet:
            payload = extract_udp_payload(packet)

            if "SEQ" in payload:
                seq = int(payload[4:9])

                if "BEGIN FLAG" in payload:
                    begin = seq
                elif "END FLAG" in payload:
                    end = seq
                else:
                    x_header_len = calculate_x_header_length(packet)
                    packets[seq] = chr(x_header_len) if x_header_len < 128 else "?"

    # print(packets[begin + 1 : end])

    for i in range(begin + 1, end):
        if packets[i] == 0:
            packets[i] = "?"

    flag = "".join(packets[begin + 1 : end])

    return flag


if __name__ == "__main__":
    pcap_file_path = sys.argv[1]
    flag = extract_flag(pcap_file_path)

    if flag:
        print("Flag:", flag)
    else:
        print("Flag not found in the pcap.")
