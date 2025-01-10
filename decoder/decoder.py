from treeClasses import *
from math import sqrt

def load_data_from_file(fileName):
    data = []
    with open(fileName, mode='rb') as file:
        fileContent = file.read()
        for byte_value in fileContent:
            byte_bin_value_str = bin(byte_value)[2:]
            zeros = ''
            if len(byte_bin_value_str) != 8:
                for _ in range(8 - len(byte_bin_value_str)):
                    zeros = zeros + '0'
            byte_bin_value_str = zeros + byte_bin_value_str
            for bit in byte_bin_value_str:
                if bit == '0':
                    data.append(False)
                elif bit == '1':
                    data.append(True)
                else:
                    raise TypeError("The file you provided is not of .bin type")
    return data

def bin_data_to_int(bin):
    number = 0
    power = len(bin) - 1
    for bit_bool in bin:
        if bit_bool:
            bit = 1
        else:
            bit = 0
        number = number + (bit * (2 ** power))
        power = power - 1
    return number

def decode(data):
    out = []
    i = 0
    symbol_tree = Tree()
    p = ''
    while i < len(data) and len(out) < 512 ** 2:
        current_node = symbol_tree.nodes[-1]
        while(type(current_node) is not ExternalNode and current_node is not None):
            bit = data[i]
            i += 1
            if not bit:
                current_node = current_node.link0
            else:
                current_node = current_node.link1
        if (type(current_node) is RootNode and current_node.link0 is None and current_node.link1 is None) or (type(current_node) is ExternalNode and current_node.value is None) or current_node is None: # jeśli NYT
            p = bin_data_to_int(data[i : i + e])
            i += e
        else:
            p = current_node.value
        symbol_tree.update_tree(p)
        out.append(p)
    return out
            
def write_to_pgm_file(data, fileName):
    side_length = sqrt(len(data))
    pgmHeader = 'P5' + '\n' + str(int(side_length)) + ' ' + str(int(side_length)) + '\n' + str(255) +  '\n'
    fout=open(fileName, 'wb')
    file_header_byte = bytearray(pgmHeader,'utf-8')
    fout.write(file_header_byte)
    fout.write(bytearray(data))
    fout.close()

print('Please enter a valid file name ending with .bin with data to decompress')
fileNameIN = input()
print('Please enter a valid file name ending with .pgm to write the decompressed data to')
fileNameOUT = input()
raw_data = load_data_from_file(fileNameIN)
decoded_data = decode(raw_data)
write_to_pgm_file(decoded_data, fileNameOUT)
