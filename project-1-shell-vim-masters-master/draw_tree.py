import graphviz


file = open('log.txt', 'r')

edge_list = []

lines = file.readlines()
filename = ""

# Create a Digraph object
dot = graphviz.Digraph(comment='Hierarchy Tree')

for line in lines:
    new_line = line.split()
    new_line = [x.replace(",", "") for x in new_line]
    print(new_line)
    if(new_line.count("root_pid:") > 0):

         node_id = new_line[new_line.index('root_pid:')+1]
         filename = new_line[new_line.index('output_file:')+1]
         dot.node(node_id, "Root Node", style="filled", fillcolor="yellow")
    
    elif(new_line.count('Child') > 0):
        print("here")
        node_id = new_line[new_line.index('PID:')+1]
        node_name = new_line[new_line.index('Name:')+1]
        parent_id = new_line[new_line.index('PID:', new_line.index('Name:')+1)+1]
        dot.node(node_id, node_name)
        edge_tuple = (parent_id, node_id)
        edge_list.append(edge_tuple)

    #print(new_line)

print(edge_list)
dot.edges(edge_list)

# Save the output as a PNG file (or choose another format)
dot.render(filename, format='png', cleanup=True)

