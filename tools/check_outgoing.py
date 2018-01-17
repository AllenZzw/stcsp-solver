'''
USage:

Type 

python check_outgoing.py -h

for detailed information
'''

from sys import argv
import io
import argparse



parser = argparse.ArgumentParser()
parser.add_argument('-i','--input',help='input file name')

#parser.add_argument('-o','--output',help='output file name')

parser.add_argument('-a1','--adversarial1',help='adversarial1 flag', action = 'store_true')

parser.add_argument('-a2','--adversarial2',help='adversarial2 flag', action = 'store_true')

args = parser.parse_args()

input_file = args.input

def adv1():
	all_nodes = set()
	print('Check adversarial1 solution')
	mydict = {}
	with io.open(input_file,"r") as input:
		print(input.readline().strip())
		for line in input:
			if '->' in line:
				pos1 = line.find('->')
				
				pos2 = line.find('[label')
				
				source = int(line[:pos1])
				
				des = int(line[pos1+2:pos2])
				
				all_nodes.add(source)
				all_nodes.add(des)
				action2 = int(line.split(',')[5])
				
				if source in mydict:
					mydict[source].add(action2) 
				else:
					mydict[source] = {action2}
				
	yes = True	
	for k,v in mydict.items():
		if len(v) < 4:
			print("No strong solution at the subgame rooted at",  k ,": unless opponent does", v)
			yes = False
			
	print(len(all_nodes), "nodes after pruning")		
	
	if yes:
		print("This is a adversarial1 solution")
	

def adv2():
	all_nodes = set()
	print('Check adversarial2 solution')
	mydict = {}
	with io.open(input_file,"r") as input:
		print(input.readline())
		for line in input:
			if '->' in line:
				pos1 = line.find('->')
				
				pos2 = line.find('[label')
				
				source = int(line[:pos1])
				
				des = int(line[pos1+2:pos2])
				
				all_nodes.add(source)
				all_nodes.add(des)
				
				action2 = int(line.split(',')[5])
				action1 = int(line.split(',')[6])
				if source not in mydict: 
					mydict[source] = {action1:{action2}}
				elif action1 in mydict[source]:
					mydict[source][action1].add(action2)
				else:
					mydict[source][action1] = {action2}
				
		
	for source,v in mydict.items():
		yes = False
		for act1, acts2_set in v.items():
			if len(acts2_set) == 4:
				yes = True
		if not yes:
			print("At node {0}, there is no viable simultaneous action".format(source))
			
	print(len(all_nodes), "nodes after pruning")	
	
	
	

if __name__ == '__main__':
	if args.adversarial1:
		adv1()
	
	if args.adversarial2:
		adv2()

		
