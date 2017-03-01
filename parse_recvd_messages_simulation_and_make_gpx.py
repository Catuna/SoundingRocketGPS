lines = "";
with open("meldingFraNyttelast.txt", "r") as f:
    lines = f.readlines();

values = [];
totalSize = 0;
for i in range(0,len(lines)):
    values.append(int((lines[i].rstrip("\n"))));
    totalSize += 1;
print("Total number of packages recv: " + str(totalSize))


messages=[];#Whole messages is the same as 13 uint8_t's
words = []; #      one word is the same as 1  uint8_t/single message/package.
remainingWords = 0;
totalLosses = 0;

#This will extract the messages
for i in range(0, len(values)):   
    if remainingWords > 0:
        #We are currently reading a message
        words.append(values[i]);
        remainingWords-=1;
        if(remainingWords == 0):
            #We have read the entire message
            messages.append(words);
            words = [];
    else:
        #we are looking for the start of a new message
        if values[i] == 255:
            remainingWords = 13;


#Here we are expanding one word(=uint8_t) into 2 chars.
messagesAsChars=[]

def validateValue(a, b):
    if(a >= 0 and a <= 9):
        return a;
    elif (a == 13):
        return -3; # a minus sign
    elif(a == 14):
        print ("Error indicator found. Setting value to 0");
        return 0;
    else:
        print(str(b) + "  Unexpected error. Value is not 0-9 or 14 or 13. Setting it to 0");
        return 0;

#print(messages)

for i in range(0, len(messages)):
    tmpMessage = [];
    for j in range(0, len(messages[i])):
        word = messages[i][j];
        char1 = chr(validateValue((word >> 4), i) + 48);
        char2 = chr(validateValue((word & int('00001111',2)), i)+48);
        tmpMessage.append(char1);
        tmpMessage.append(char2);
    messagesAsChars.append(tmpMessage);
    
    

       

#Reconstruct the information.
        
#             latitude, longitude, quality ind., numofsat, altitude
#field_sizes:   8           9           1           2           6
print("\n\n\n")
#print(messagesAsChars)
interpretedMessages = [];
tmpmsg = [];
print(len(messagesAsChars))
for i in range(0, len(messagesAsChars)):
    #print(messagesAsChars)
    msg = messagesAsChars[i];
    #print(msg)
    #print(len(msg))
    #Hard coding was the easiest way.
    #interpretedMessages[i].append() = [float(msg[0]+msg[1]+msg[2]+msg[3]+msg[4]+'.'+msg[5]+msg[6]+msg[7]+msg[8]),
    #                              float(str(msg[9])+str(msg[10])+str(msg[11])+str(msg[12])+str(msg[13])+'.'+str(msg[14])+str(msg[15])+str(msg[16])+str(msg[17])),
    #                              int(msg[18]),
    #                              int(msg[19] + msg[20]),
    #                              float(msg[21]+msg[22]+msg[23]+msg[24]+'.'+msg[25])];
    #print(float(msg[0]+msg[1]+msg[2]+msg[3]+msg[4]+'.'+msg[5]+msg[6]+msg[7]+msg[8]))
    tmpmsg.append(float(msg[0]+msg[1]+msg[2]+msg[3]+'.'+msg[4]+msg[5]+msg[6]+msg[7]))
    tmpmsg.append(float(str(msg[8]) + str(msg[9])+str(msg[10])+str(msg[11])+str(msg[12])+'.'+str(msg[13])+str(msg[14])+str(msg[15])+str(msg[16])))
    tmpmsg.append(int(msg[17]))
    tmpmsg.append(int(msg[18] + msg[19]))
    tmpmsg.append(float(msg[20]+msg[21]+msg[22]+msg[23]+'.'+msg[24]+msg[25]))
    interpretedMessages.append(tmpmsg)
    tmpmsg = [];
    

print(interpretedMessages[0])

#Store the interpreted messages for later use.
#with open("interpretedMessages.txt", "w") as f:
#    for i in range(0, len(interpretedMessages)):
#        for j in range(0, len(interpretedMessages[i])):
#            #print(j)
#            f.write(str(interpretedMessages[i][j]));
#            if (j < len(interpretedMessages[i])):
##                f.write(',');
#       if i < len(interpretedMessages)-1:
#            f.write('\n');



import gpxpy
import gpxpy.gpx


gpx = gpxpy.gpx.GPX()

# Create first track in our GPX:
gpx_track = gpxpy.gpx.GPXTrack()
gpx.tracks.append(gpx_track)

# Create first segment in our GPX track:
gpx_segment = gpxpy.gpx.GPXTrackSegment()
gpx_track.segments.append(gpx_segment)
#lat lon elevation

def stupidFormat(a):
    a = str(a);
    dotIndex = a.index('.')-2;
    b = a[0:dotIndex]
    c = a[dotIndex:-1]
    return float(b) + float(c)/60;
    


for i in range(0, len(interpretedMessages)):
    gpx_segment.points.append(gpxpy.gpx.GPXTrackPoint(stupidFormat(interpretedMessages[i][0]), stupidFormat(interpretedMessages[i][1]), elevation=interpretedMessages[i][4]))

with open("locations.gpx", "w") as f:
    f.write(gpx.to_xml())






















                



    
