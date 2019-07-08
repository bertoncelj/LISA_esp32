
from pyexcel_ods import save_data
from collections import OrderedDict
'''
data = OrderedDict() # from collections import OrderedDict
data.update({"Sheet 1": [[2, 2, 3], [4, 5, 6]]})
data.update({"Sheet 2": [["row 1", "row 2", "row 3"]]})
save_data("your_file.ods", data)

'''
list_elms = [[ "Uavg", "ANGdiff", "U1", "U2", "ANG1", "ANG2", "U3", "U4", "Battery", "Temp"]]
countTime = 0
global_date = " "

def reconfigureDataPositions(elms, date, time):
    if elms:
        #print(elms)
        temp = elms[0]
        Ubat = elms[1]
        U1 = elms[2]
        U2 = elms[3]
        Upov = elms[4]
        ANG = elms[5]
        ANG1 = elms[6]
        ANG2 = elms[7]
        U3 = elms[8]
        U4 = elms[9]
        ANG_tot = elms[10]
        ANG3 = elms[11]
        ANG4 = elms[12]

        elems = [ Upov, ANG_tot, U1, U2,ANG1, ANG2, U3, U4, Ubat, temp]
        list_elms.append(elems) 

    
def parse_data(data, date, time):
    elems_data = data.split(";")

    for single_data in elems_data:
        elms = single_data.split()
        reconfigureDataPositions(elms, date, time) 

def main_func(line):

    if "Measure:" in line:
        content = line.split("e:")

    if "Measure " in line:
        content = line.split(" : ")

    if "Graph" in line:
        return True

    head = content[0]
    data = content[1]
    head = head.split()

    if "Measur" == head[2]:
        parse_data(data, head[0], head[1])

    if "Measure" == head[2]:
        parse_data(data, " ", " ")

########################################

fname = "cp_log.txt"
with open(fname) as f:
    content = f.readlines()
# you may also want to remove whitespace characters like `\n` at the end of each line
#content = [x.strip() for x in content]
countToTen = 0
for line in content:
    rtn = main_func(line)
    countToTen = countToTen + 1
    print(countToTen)


data = OrderedDict() 
data.update({"Sheet 1": list_elms})
data.update({"Sheet 2": [["row 1", "row 2", "row 3"]]})
save_data("your_file02.ods", data)
        



'''
data = OrderedDict() # from collections import OrderedDict
data.update({"Sheet 1": list_elms})
data.update({"Sheet 2": [["row 1", "row 2", "row 3"]]})
save_data("your_file.ods", data)

'''
'''
for line in content[0]:
    print(line)
    elems = line.split(" ")
    if elems[2] == "Graph":
        print("SKIP")

'''
