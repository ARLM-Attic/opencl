close all
clear
clc

% Plot dos pontos originais.
fid = fopen('../datasets/d1.txt');
data = textscan(fid,'%d %d',1); count = data{1};
data = textscan(fid,'%f',count*count); original_mesh = reshape(data{:},count,count);
fclose(fid);

% figure('Color',[1 1 1])
% axis([1 count 1 count 1 count])
% box on
% hold on
% mesh(1:count,1:count,original_mesh)
% hold off
% xlabel('X')
% ylabel('Y')
% zlabel('Z')

% Plot da malha rasterizada.
fid = fopen('volume.txt');
data = textscan(fid,'%d'); volume = reshape(data{:},count,count,count);
fclose(fid);

ind = find(volume(:));
sub = cell(1,3);
[sub{:}] = ind2sub(size(volume),ind);

figure('Color',[1 1 1])
axis([1 count 1 count 1 count])
box on
hold on
mesh(1:count,1:count,original_mesh)
line(sub{1},sub{3},sub{2},'LineStyle','none','Marker','s')
hold off
xlabel('X')
ylabel('Y')
zlabel('Z')