def get_decode_table(fileName):
    with open(fileName, mode='rb') as file: # b is important -> binary
        fileContent = file.read()
        data = int.from_bytes(fileContent, byteorder='little', signed=False)
        print(data)
        # num_of_symbols = int.from_bytes(fileContent[ : 1], byteorder='little', signed=False)
        # symbols_values = []
        # for i in range(1, num_of_symbols):
        #     symbols_values[i] = int.from_bytes(fileContent[i : i + 1], byteorder='little', signed=False)
        # highest_mask = int.from_bytes(fileContent[num_of_symbols : num_of_symbols + 1], byteorder='little', signed=False)

        # num_in_mask = []
        # for i in range(num_of_symbols + 1, num_of_symbols + highest_mask + 1):
        #     num_in_mask[i] = int.from_bytes(fileContent[i : i + 1], byteorder='little', signed=False)

        
get_decode_table("C:/Users/Gosia/Downloads/compressed.bin")