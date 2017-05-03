import gpxpy
import gpxpy.gpx

DIGITAL_FIELD = 10; # D0 = 9, D1 = 10
FRAME_COUNT_FIELD = 11;
inputFile = "meldingFraNyttelast.txt";
    
def extractMessages(values):
    messages=[]; # one message is the same as 13 uint8_t's
    words = []; #one word is the same as one uint8_t. (one element of the values variable)
    remainingWords = 0;

    prevCount = values[0][0]-1; #Used to detect out-of-order packets.
    
    for i in range(0, len(values)):   
        #Check if the packet is in order. Note that the counter will wrap around. max value = 2^16-1.
        count = values[i][0];
        if (count == 0): 
            if(prevCount != 65535):
                #Out of order
                remainingWords = 0;
                words = [];
        elif(count != prevCount+1):
            #Out of order
            remainingWords = 0;
            words = [];    
        else: #Packet is in order:
            if remainingWords > 0:
                #We are currently reading a message
                words.append(values[i][1]);
                remainingWords-=1;
                if(remainingWords == 0):
                    #We have read the entire message
                    messages.append(words);
                    words = [];
            else:
                #we are looking for the start of a new message
                if values[i] == 255:
                    remainingWords = 13;
    return messages;

def validateValue(a, b):
    if(a >= 0 and a <= 9):
        return a;
    elif (a == 13):
        return -3; # a minus sign
    elif(a == 14):
        #print ("Error indicator found.");
        return None;
    else:
        #print(str(b) + "  Unexpected error. Value is not 0-9 or 14 or 13.");
        return None;


def ConvertMessagesBackToChars(messages):
    messagesAsChars=[]
    errorIndices = []
    messagesWithErrors = []
    
    # Here we are expanding one word(=uint8_t) into 2 chars.
    for i in range(0, len(messages)):
        tmpMessage = [];
        for j in range(0, len(messages[i])):
            word = messages[i][j];
            
            tmp1 = validateValue((word >> 4), i);
            tmp2 = validateValue((word & int('00001111',2)), i)
            if tmp1 == None or tmp2 == None:
                # Something unexpected has happen.
                errorIndices.append(i);
                messagesWithErrors.append(messages[i]);
                tmpMessage = []
                #print(str(i) + "tmp1 or tmp2 is None")
                break; # No need to keep iterating through the rest of the message.
            else:
                char1 = chr(tmp1 + 48);
                char2 = chr(tmp2 + 48);
                tmpMessage.append(char1);
                tmpMessage.append(char2);
        
        if (len(tmpMessage) != 0):
            messagesAsChars.append(tmpMessage);
    
    return messagesAsChars, errorIndices, messagesWithErrors;
    

       


def interpretMessages(messagesAsChars):
    #             latitude, longitude, quality ind., numofsat, altitude
    #field_sizes:   8           9           1           2           6
    
    interpretedMessages = [];
    tmpmsg = [];
    
    # Reconstruct the information.
    for i in range(0, len(messagesAsChars)):
        msg = messagesAsChars[i];
        #print(str(i) + " of " + str(len(messagesAsChars)))
        latitude = float(msg[0]+msg[1]) + float(msg[2]+msg[3]+'.'+msg[4]+msg[5]+msg[6]+msg[7] )/60;
        longitude = float(msg[8] + msg[9] + msg[10]) + float(msg[11] + msg[12] + '.' + msg[13]+msg[14]+msg[15]+msg[16])/60;
        qualityIndicator = int(msg[17]);
        numberOfSats = int(msg[18] + msg[19]);
        altitude = float(msg[20]+msg[21]+msg[22]+msg[23]+'.'+msg[24]+msg[25])
        tmpmsg.append(latitude);
        tmpmsg.append(longitude);
        tmpmsg.append(qualityIndicator);
        tmpmsg.append(numberOfSats);
        tmpmsg.append(altitude);
        interpretedMessages.append(tmpmsg)
        tmpmsg = [];

    return interpretedMessages;


def deleteMessagesWithBadQualityIndicator(interpretedMessages):
    indicesToBeDeleted = [];
    
    # Find all messages that have an error at the quality indicator 
    # TODO: what is an unwanted quality indicator value??
    for i in range(0, len(interpretedMessages)):
        if (interpretedMessages[i][2] == 88):
            indicesToBeDeleted.append(i);
    
    # Delete the found messages.
    for i in range(0, len(indicesToBeDeleted)):
        del interpretedMessages[indicesToBeDeleted[i]];
    return interpretedMessages;


def createGPX(interpretedMessages, filename):
    gpx = gpxpy.gpx.GPX()
    
    # Create first track in our GPX:
    gpx_track = gpxpy.gpx.GPXTrack()
    gpx.tracks.append(gpx_track)
    
    # Create first segment in our GPX track:
    gpx_segment = gpxpy.gpx.GPXTrackSegment()
    gpx_track.segments.append(gpx_segment)
    #lat lon elevation
    
    
    for i in range(0, len(interpretedMessages)):
        gpx_segment.points.append(gpxpy.gpx.GPXTrackPoint(interpretedMessages[i][0], interpretedMessages[i][1], elevation=interpretedMessages[i][4]))
    
    with open(filename, "w") as f:
        f.write(gpx.to_xml())



def main():
    # Read the raw data.
    rawData = "";
    with open(inputFile, "r") as f:
        rawData = f.readlines();

    # Convert each element into an integer and also strip the \n at the end
    values = []; # Format: [[counterVal,packet],[counterVal,packet]] 
    for i in range(0,len(rawData)):
                      #Counter                                                     #one word
        values.append([int((lines[i].rstrip("\n")).split(",")[FRAME_COUNT_FIELD]), int((lines[i].rstrip("\n")).split(",")[DIGITAL_FIELD])]);


    messagesAsUint8 = extractMessages(values);
    messagesAsChars, errorIndices, messagesWithErrors = ConvertMessagesBackToChars(messagesAsUint8);
    interpretedMessages = interpretMessages(messagesAsChars);
    interpretedMessages = deleteMessagesWithBadQualityIndicator(interpretedMessages); 
    for elem in interpretedMessages:
        print (elem)

    #TODO: Handle the errorIndices and messagesWithErrors. Not sure what you want to do with them.
    #TODO: Check if the values make sence (i.e. not travelled insanely far). Remember that we might lose and regain "fix"...
    createGPX(interpretedMessages,"locations.gpx")

main();
