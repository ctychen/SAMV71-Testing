function read_serial()
% List available COM ports
ports = serialportlist;
num_ports = length(ports);

% Ask the user to input the mode
mode = input('Please enter the mode you want to use (0 for idle, 1 for bin, 2 for event-by-event): ')
modeChar = uint8(mode);
% modeChar = num2str(mode)

count = 0;
switch mode
    case 1
        count = 1+4+2+64+3+2-1;
        
    case 2
        count = 1+4+2+2+4+3-1;

    case 0
        count = 1+4+2+64+3;
end

% count = count - 1;

dataArray = [];
i = 1;
j = 1;
for k = 1:num_ports
    fprintf('%d. %s\n', k, ports(k))
end

% Ask the user to input the COM port number
port_num = input('Please enter the number of the COM port you want to use: ');
device = ports(port_num);

baud_rate = 115200;
s = serialport(device, baud_rate);
% s.BytesAvailableFcnCount = count;
% configureCallback(s,"byte",count,@my_callBack);


% Clear input buffer
flush(s);

message = ['S',modeChar];

% Write 'S' to the serial port
write(s, message, "uint8");
% write(s, modeChar,"uint8");


% At this point, data should be aligned
global data;
while true % repeat the loop
    data = read(s,1,"uint8");
%     fprintf("Looking for data start byte... ")
    while data ~= mode
        data = read(s,1,"uint8");
    end
%     fprintf("Successfully found start byte! \n")
%     configureCallback(s, "byte", count, @(src, ~)my_callBack(src, count))
    % store the command
    dataArray(i,j) = data;
    j = j + 1;
    data = [];
    
    while s.NumBytesAvailable < count
%         fprintf("current Num of Data: %d\n",s.NumBytesAvailable)
    end
%     data1 = readline(s)
    data = read(s,count,"uint8");
    
    currentIndex = 1;

%     % one byte for command
%     dataArray(i,j) = data(currentIndex);
%     j = j+1;
%     currentIndex = currentIndex + 1;

    % throw away three bytes
    currentIndex = currentIndex + 3;
    % four bytes for seconds
    int32data = uint32(data(currentIndex)) + bitshift(uint32(data(currentIndex+1)), 8) ...
        + bitshift(uint32(data(currentIndex+2)), 16) + bitshift(uint32(data(currentIndex+3)), 24);
    dataArray(i,j) = int32data;
    j = j + 1;
    currentIndex = currentIndex + 4;

    % Read 2 bytes for miliseconds
    int16data = uint16(data(currentIndex)) + bitshift(uint16(data(currentIndex+1)), 8);
    dataArray(i,j) = int16data;
    j = j + 1;
    currentIndex = currentIndex + 2;

    if mode == 2 % Event-by-event mode
        % Read 2 byte for microseconds
        int16data = uint16(data(currentIndex)) + bitshift(uint16(data(currentIndex+1)), 8);
        dataArray(i,j) = int16data;
        j = j + 1;
        currentIndex = currentIndex + 2;
        % Read 4 bytes for detector data
        for index = 0:3
            dataArray(i,j) = data(currentIndex+index);
            j = j + 1;
        end
        currentIndex = currentIndex + 4;
    else % Bin mode
        % throw away 2 more bytes for mili empy space
        currentIndex = currentIndex + 2;
        % Read 64 bytes for bin data
        for index = 0:63
            dataArray(i,j) = data(currentIndex+index);
            j = j + 1;
        end
        currentIndex = currentIndex + 64;
    end

    for l = 1:j-1
        fprintf('%d, ',dataArray(i,l));
    end
    fprintf('\n');

    i = i + 1;
    j = 1;

    write(s, message, "uint8");
end

end

% function my_callBack(src, count)
% global data;
% data = read(src,count,"uint8");
% end


