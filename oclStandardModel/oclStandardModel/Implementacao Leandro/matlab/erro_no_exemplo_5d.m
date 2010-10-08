close all
clear
clc

P = [ 1  3  4];
T = [ 2  4  5];
U = [ 3  8 10];

[t u] = ndgrid(-100:100,-100:100);

s = [repmat(P,numel(t),1)+t(:)*T+u(:)*U ones(numel(t),1)];

F = [4 6 0 10]';
G = [cross(T,U) -dot(cross(T,U),P)]';

sum([s*F s*G]==0,1)