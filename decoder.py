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

class Node:
    def show_subtree(self, indention=''):
        description = ''
        description += str(self)
        if self.link1 is not None and self.link0 is not None:
            description += ':'
            child_number = 0
            children = [self.link0, self.link1]
            for child in children:
                description += '\n'
                description += indention
                if len(children) - 1 == child_number:
                    description += '\u2514\u2500> '
                else:
                    description += '\u251c\u2500> '
                if type(child) is InternalNode:
                    if len(children) - 1 == child_number:
                        indention += '    '
                    else:
                        indention += '\u2502   '
                    description += child.show_subtree(indention)
                    indention = indention[:-4]
                else:
                    description += str(child)
                child_number += 1
        return description

class RootNode(Node):
    def __init__(self, link0, link1):
        self.link0 = link0
        self.link1 = link1
        self.weight = 0
        self.number = 0
    def __str__(self):
        return ('Root ' + str(self.weight))

    

class InternalNode(RootNode):
    def __init__(self, link0, link1, parent, weight):
        super().__init__(link0, link1)
        self.parent = parent
        self.weight = weight
    def __str__(self):
        return ('N ' + str(self.weight))

class ExternalNode(Node):
    def __init__(self, parent, weight, value):
        self.parent = parent
        self.weight = weight # 0 dla NYT
        self.value = value # None dla NYT
        self.number = 0 # 0 dla NYT
    def __str__(self):
        if self.value is None:
            return 'NYT'
        return (str(self.value) +' ' + str(self.weight))

def swapNodes(node1, node2):
    n1 = node1.number
    node1.number = node2.number
    node2.number = n1
    p1 = node1.parent
    p2 = node2.parent
    p10 = False
    p11 = False
    p20 = False
    p21 = False
    if p1.link0 == node1:
        p10 = True
    elif p1.link1 == node1:
        p11 = True

    if p2.link0 == node2:
        p20 = True
    elif p2.link1 == node2:
        p21 = True

    if p10:
        p1.link0 = node2
    elif p11:
        p1.link1 = node2
    if p20:
        p2.link0 = node1
    elif p21:
        p2.link1 = node1

    node1.parent = p2
    node2.parent = p1


class Tree:
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
            current_node = leaf
            # tu potem wracamy
            node_with_biggest_num_in_group = max((node for node in self.nodes if node.weight == current_node.weight and type(node) != RootNode and type(current_node) != RootNode and node != current_node.parent), key=lambda node: node.number, default=None)
            if node_with_biggest_num_in_group and not current_node.number == node_with_biggest_num_in_group.number:
                swapNodes(node_with_biggest_num_in_group, current_node)

            current_node.weight = current_node.weight + 1
        else: #pierwsze wystąpienie symbolu
            for node in self.nodes:
                if node.number == 0:
                    old_NYT = node
            new_symbol_node = ExternalNode(None, 1, new_symbol)
            new_symbol_node.number = 1
            new_NYT = ExternalNode(None, 0, None)
            new_NYT.number = 0
            if type(old_NYT) is ExternalNode:
                old_NYT_transformed = InternalNode(new_symbol_node, new_NYT, old_NYT.parent, 1)
                if old_NYT.parent.link0 == old_NYT:
                    old_NYT.parent.link0 = old_NYT_transformed
                elif old_NYT.parent.link1 == old_NYT:
                    old_NYT.parent.link1 = old_NYT_transformed
            else:
                old_NYT_transformed = RootNode(new_symbol_node, new_NYT)
                old_NYT_transformed.weight = 1
            old_NYT_transformed.number = 2
            new_NYT.parent = old_NYT_transformed
            new_symbol_node.parent = old_NYT_transformed
            self.nodes[:] = [node for node in self.nodes if node.weight != 0] # usuń stary NYT
            for node in self.nodes:
                node.number = node.number + 2
            self.nodes.append(old_NYT_transformed)
            self.nodes.append(new_symbol_node)
            self.nodes.append(new_NYT)
            self.nodes = sorted(self.nodes, key=lambda node: node.number)
            current_node = old_NYT_transformed
        
        while type(current_node) is not RootNode:
            current_node = current_node.parent
            node_with_biggest_num_in_group = max((node for node in self.nodes if node.weight == current_node.weight and type(node) != RootNode and type(current_node) != RootNode  and node != current_node.parent), key=lambda node: node.number, default=None)
            if node_with_biggest_num_in_group and not current_node.number == node_with_biggest_num_in_group.number and type(current_node) is not RootNode:
                swapNodes(node_with_biggest_num_in_group, current_node)
            current_node.weight = current_node.weight + 1
        self.nodes = sorted(self.nodes, key=lambda node: node.number)

        root = self.nodes[-1]
        tree_str = root.show_subtree()
        print(tree_str)
    
    

        

symbol_tree = Tree()
symbol_tree.update_tree('A')
symbol_tree.update_tree('A')
symbol_tree.update_tree('A')
symbol_tree.update_tree('B')
symbol_tree.update_tree('D')
symbol_tree.update_tree('D')
symbol_tree.update_tree('H')
symbol_tree.update_tree('P')
symbol_tree.update_tree('B')
symbol_tree.update_tree('A')
symbol_tree.update_tree('B')
symbol_tree.update_tree('D')
symbol_tree.update_tree('U')
symbol_tree.update_tree('U')
symbol_tree.update_tree('U')
symbol_tree.update_tree('H')
