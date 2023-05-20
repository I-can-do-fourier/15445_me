import lldb

def print_trienode(trienode, indent=""):
    print(f"{indent}is_value_node: {trienode.is_value_node_}")

    for char, child_node in trienode.children_.GetChildMemberWithName("items"):
        print(f"{indent}Child: {char}")
        print_trienode(child_node, indent + "  ")
