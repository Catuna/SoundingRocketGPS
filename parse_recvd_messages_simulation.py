lines = "";
with open("medlingFraNyttelast.txt", "r") as f:
    lines = f.readlines();

values = [];
totalSize = 0;
for i in range(0,len(lines)):
    values.append(int((lines[i].rstrip("\n"))));
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
    if remainingWords > 0:
        #We are currently reading a message
        words.append(values[i]);
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


#Here we are expanding one word(=uint8_t) into 2 chars.
messagesAsChars=[]

def validateValue(a):
    if(a >= 0 and a <= 9):
        return a;
    elif(a == 15):
        print ("Error indicator found. Setting value to 0");
        return 0;
    else:
        print("Unexpected error. Value is not 0-9 or 15. Setting it to 0");
        return 0;


for i in range(0, len(messages)):
    for j in range(0, len(messages[i])):
        word = messages[i][j];
        char1 = chr(validateValue((word >> 4) + 48));
        char2 = chr(validateValue((word & int('00001111',2))+48));
        messagesAsChars.append(char1);
        messagesAsChars.append(char2);

       

#Reconstruct the information.
        
#             latitude, longitude, quality ind., numofsat, altitude
#field_sizes:   8           9           1           2           6             
interpretedMessages = [];
for i in range(0, len(messagesAsChars)):
    msg = messagesAsChars[i];
    #Hard coding was the easiest way.
    interpretedMessages[i] = [float(msg[0]+msg[1]+msg[2]+msg[3]+msg[4]+'.'+msg[5]+msg[6]+msg[7]+msg[8],
                                  +msg[9]+msg[10]+msg[11]+msg[12]+msg[13]+'.'+msg[14]+msg[15]+msg[16]+msg[17]),
                                  int(msg[18]),
                                  int(msg[19] + msg[20]),
                                  float(msg[21]+msg[22]+msg[23]+msg[24]+msg[25]+'.'+msg[26])];
            

#Store the interpreted messages for later use.
with open("interpretedMessages.txt", "w") as f:
    for i in range(0, len(interpretedMessages)):
        for j in range(0, len(interpretedMessages[i])):
            f.write(str(interpretedMessages[i][j]));
            if (j < len(interpretedMessages[i])):
                f.write(',');
        if i < len(interpretedMessages)-1:
            f.write('\n');
                



    
