lines = "";
with open("ntnu2016_ToRussiaWithLove.txt", "r") as f:
    lines = f.readlines();

values = [];
totalSize = 0;
for i in range(1,len(lines)):
    values.append([int((lines[i].rstrip("\n")).split(",")[11]), int((lines[i].rstrip("\n")).split(",")[10])]);
    totalSize += 1;
print("Total number of packages recv: " + str(totalSize))


numberOf255Words = 0;
messages=[];#Whole messages is the same as 13 uint8_t's
words = []; #      one word is the same as 1  uint8_t/single message/package.
remainingWords = 0;
totalLosses = 0;
prevCount = values[0][0]-1;

#This will extract the messages
for i in range(0, len(values)):
    #Check if the message is in order. Note that the counter will wrap around.
    count = values[i][0];
    if (count == 0): #we need to have 2 seperate tests. else the elif below may trigger when it shouldn't.
        if(prevCount != 65535):
            #Out of order
            totalLosses +=1;
            remainingWords = 0;
            numberOf255Words = 0;
            words = [];
    elif(count != prevCount+1):
        #Out of order
        totalLosses +=1;
        remainingWords = 0;
        numberOf255Words = 0;
        words = [];    
    else: #Message is in order:

        
        if remainingWords > 0:
            #We are currently reading a message
            words.append(values[i][1]);
            remainingWords-=1;
            if(remainingWords == 0):
                #We have read the entire message
                numberOf255Words = 0;
                messages.append(words);
                words = [];
        else:
            #we are looking for the start of a new message
            if values[i][1] == 255 and numberOf255Words == 0:
                numberOf255Words+=1;
            elif values[i][1] == 255 and numberOf255Words == 1:
                #start of message found
                numberOf255Words=0;
                remainingWords = 13;
                
            elif numberOf255Words ==1 and values[i][1] != 255:
                #It was just a value 255 and not the start of a message
                numberOf255Words=0;

        
    prevCount = count;


print ("Total misplaced counters: " + str(totalLosses));

#Here we are expanding one word(=uint8_t) into 2 chars.
messagesAsChars=[]
for i in range(0, len(messages)):
    for j in range(0, len(messages[i])):
        word = messages[i][j];
        char1 = chr((word >> 4) + 48);
        char2 = chr((word & int('00001111',2))+48);
        messagesAsChars.append(char1);
        messagesAsChars.append(char2);

       

#Reconstruct the information.
        
#             latitude, longitude, quality ind., numofsat, altitude
#field_sizes:   8           9           1           2           6             
messagesInterpreted = [];
for i in range(0, len(messages)):
    msg = messages[i];
    #Hard coding was the easiest way.
    messagesInterpreted[i] = [float(msg[0]+msg[1]+msg[2]+msg[3]+msg[4]+'.'+msg[5]+msg[6]+msg[7]+msg[8],
                                  +msg[9]+msg[10]+msg[11]+msg[12]+msg[13]+'.'+msg[14]+msg[15]+msg[16]+msg[17]),
                                  int(msg[18]),
                                  int(msg[19] + msg[20]),
                                  float(msg[21]+msg[22]+msg[23]+msg[24]+msg[25]+'.'+msg[26])]
            

#Store the interpreted messages for later use.
with open("messages.txt", "w") as f:
    for i in range(0, len(messagesInterpreted)):
        for j in range(0, len(messagesInterpreted[i])):
            f.write(str(messagesInterpreted[i][j]));
            if (j < len(messagesInterpreted[i])):
                f.write(',');
        if i < len(messagesInterpreted)-1:
            f.write('\n');
                



    
