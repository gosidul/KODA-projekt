e = 8
r = 0

def load_data_from_file(self, fileName):
    data = []
    with open(fileName, mode='rb') as file:
        fileContent = file.read()
        for byte_value in fileContent:
            byte_bin_value_str = bin(byte_value)[2:]
            for bit in byte_bin_value_str:
                if bit == '0':
                    data.append(0)
                elif bit == '1':
                    data.append(1)
                else:
                    raise TypeError("The file you provided is not of .bin type")
    return data

class RootNode:
    def __init__(self, link0, link1):
        self.link0 = link0
        self.link1 = link1
        self.number = 0

class InternalNode(RootNode):
    def __init__(self, link0, link1, parent, weigth):
        super.__init__(self, link0, link1)
        self.parent = parent
        self.weigth = weigth

class ExternalNode:
    def __init__(self, parent, weigth, value):
        self.parent = parent
        self.weigth = weigth # 0 dla NYT
        self.value = value # None dla NYT
        self.number = 0

class tree:
    def __init__(self):
        root = RootNode(None, None)
        self.nodes = [root]
    def update_tree(self, new_symbol):
        first_occurence = True
        for node in self.nodes:
            if type(node) is ExternalNode and node.value is not None and node.value == new_symbol:
                leaf = node
                first_occurence = False
        if not first_occurence:
            current_node = leaf.parent
            # tu potem wracamy
            node_with_biggest_num_in_group = max((node for node in self.nodes if node.weight == current_node.weigth), key=lambda node: node.number, default=None)
            if not current_node.number == node_with_biggest_num_in_group.number:
                biggest_number = node_with_biggest_num_in_group.number
                node_with_biggest_num_in_group.number = current_node.number
                current_node.number = biggest_number
            current_node.weight = current_node.weight + 1
        else: #pierwsze wystąpienie symbolu
            old_NYT = (node for node in self.nodes if node.number == 0)
            new_symbol_node = ExternalNode(None, 1, new_symbol)
            new_symbol_node.number = 1
            new_NYT = ExternalNode(None, 0, None)
            new_NYT.number = 0
            if type(old_NYT) is not RootNode:
                old_NYT_transformed = InternalNode(new_symbol_node, new_NYT, old_NYT.parent, 1)
            else:
                old_NYT_transformed = RootNode(new_symbol_node, new_NYT)
            old_NYT_transformed.number = 2
            new_NYT.parent = old_NYT_transformed
            new_symbol_node.parent = old_NYT_transformed
            self.nodes[:] = [node for node in self.nodes if node.number != 0] # usuń stary NYT
            for node in self.nodes:
                node.number = node.number + 2
            self.nodes.append(old_NYT_transformed)
            self.nodes.append(new_symbol_node)
            self.nodes.append(new_NYT)
            current_node = old_NYT_transformed
        
        while type(current_node) is not RootNode:
            node_with_biggest_num_in_group = max((node for node in self.nodes if node.weight == current_node.weigth), key=lambda node: node.number, default=None)
            if not current_node.number == node_with_biggest_num_in_group.number:
                biggest_number = node_with_biggest_num_in_group.number
                node_with_biggest_num_in_group.number = current_node.number
                current_node.number = biggest_number
            current_node.weight = current_node.weight + 1
        



# def get_data(fileName):
#     with open(fileName, mode='rb') as file:
#         fileContent = file.read()
#         data = 0
#         shift = (len(fileContent) - 1) * 8 
#         for byte_value in fileContent:
#             byte_bin_value = bin(byte_value)
#             data = data + int(byte_value) * (2 ** shift)
#             shift = shift - 8
#         return data

# def get_bin_str(data):
#     binary_data = bin(data)[2:]
#     zeros = ''
#     for _ in range((8 - (len(binary_data) % 8)) % 8):
#         zeros = zeros + '0'
#     binary_data = zeros + binary_data
#     return binary_data

# def get_bool_table()

# bin_data = get_bin_str(get_data("compressed.bin"))
# print(bin_data)
# # print(bin(get_data("compressed.bin")))


