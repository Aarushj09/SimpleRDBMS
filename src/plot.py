import matplotlib.pyplot as plt
plt.style.use('seaborn-deep')

file_pref = '../data/range_'
for fanout in range(3, 8, 2):
    filename = file_pref + str(fanout) + '.txt'
    bptree = dict()
    heap = dict()
    with open(filename) as f:
        for line in f:
            acc = line.split()
            if acc[0] not in bptree.keys():
                bptree[acc[0]] = 0
            if acc[1] not in heap.keys():
                heap[acc[1]] = 0
            bptree[acc[0]] += 1
            heap[acc[1]] += 1
        plt.bar(list(bptree.keys()), bptree.values(), label='B+ Tree')
        plt.bar(list(heap.keys()), heap.values(), label='Heap')
        plt.legend(loc='upper left')
        plt.xlabel('Number of Block Accesses')
        plt.ylabel('Count of Queries')
        plt.title('Plot of Block Accesses for FANOUT = ' + str(fanout))
        plt.show()

    
            