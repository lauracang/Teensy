close all
disp('Opening port..')
% you might have to change the port number
com = CreateSerialConnection('COM4',115200);

InputBufferSize=10;
total_samples=10; % the script reads this many samples and then stops.
power=15; % # of powerpins on board
ground=15; % # of groundpins on board
%power10=10;
%ground10=10;
%power5=5;
%ground5=5;

buf=(power*ground*2)+9; % size of buffer = grid points x 2 (upper and lower bound) + header (4), timestamp (4) and checksum (1)

data=zeros(power,ground,total_samples); % the pressure map data, arranged in a grid. the third dimension is time.
timestamp=zeros(total_samples,1); % time values
disp('Reading data')



k=1;
while(k<total_samples)

    % wait until all data is available
    while (get(com,'BytesAvailable')<buf)
    end

    % check for the header
    while (sum(fread(com,4)==[255 255 255 255]')<4)
    end

    % wait for the rest of the data
    while (get(com,'BytesAvailable')<buf-4)
    end

    % read the timestamp (4 bytes)
    t=fread(com,1,'uint32');
    timestamp(k)=t;

    % read the pressure map (powerpins x groundpins = bytes)
    value=fread(com, [ground power],'uint16');
    %value10=fread(com,[ground10 power10],'uint16');
    %value5=fread(com,[ground5+10 power5+10],'uint16');
    %display(t);
    diff=1024-value;
    disp(diff);

    %diff10=1024-value10;
    %disp(diff10);
    %diff5=1024-value5;
    %disp(diff5);


    % read the lowest 8 bits of the checksum
    true_checksum=fread(com,1,'uint8');

    % calculate the checksum, check if it is correct
    calc_checksum=sum(sum(value))+t;
    calc_checksum=mod(calc_checksum,256);

    if (true_checksum ~= calc_checksum)
        disp('Bad data!')
        continue;
    end


    % if everything's fine, put data in arrays
    value=value';
    %value10=value10';
    %value5=value5';
    data=value(:,1:ground);
    %data10=value10(:,1:ground10);
    %data5=value5(:,1:gound5);

    k=k+1;

    dlmwrite('130cw.csv',data,'-append');

    % draw the pressure map if we have time
    if get(com,'BytesAvailable')<buf
        imagesc(data,[0 900])
        %imagesc(data10,[0 900])
        %imagesc(data5,[0 900])
       % colormap(gray)
        axis equal;
        axis off;
        pause(0.001)

      %  disp(data);
   % else
       %  disp('No time to draw!');
    end


end

fclose(com);
delete(com);
