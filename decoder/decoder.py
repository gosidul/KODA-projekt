from treeClasses import *

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
                    data.append(0)
                elif bit == '1':
                    data.append(1)
                else:
                    raise TypeError("The file you provided is not of .bin type")
    return data

def bin_data_to_int(bin):
    number = 0
    power = len(bin) - 1
    for bit in bin:
        number = number + (bit * (2 ** power))
        power = power - 1
    return number


data = load_data_from_file('compressed.bin')
out = []
i = 0
symbol_tree = Tree()
p = ''
while i != len(data) - 1:
    for node in symbol_tree.nodes:
        if type(node) is RootNode and type(node) is not InternalNode:
            current_node = node
            break
    if p == 'D':
        pass # miejsce na płapkę
    while(type(current_node) is not ExternalNode and current_node is not None):
        bit = data[i]
        i += 1
        if bit == 0:
            current_node = current_node.link0
        elif bit == 1:
            current_node = current_node.link1
    if (type(current_node) is RootNode and current_node.link0 is None and current_node.link1 is None) or (type(current_node) is ExternalNode and current_node.value is None) or current_node is None: # jeśli NYT
        p = chr(bin_data_to_int(data[i : i + e]))
        # w ogólnym przypadku tu powinien być dodatkowy warunek ale on zawsze będzie spełniony bo r = 0
        out.append(p)
        i += e
    else:
        p = current_node.value
    symbol_tree.update_tree(p)
        
