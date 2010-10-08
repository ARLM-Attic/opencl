close all
clear
clc

%           p1  p2  p3
%vertices = [ 7   2  9
%             5   4  5
%             6   9  4];

%vertices = [ 1   0  1
%             1   0  2
%             0   1  1];
         
vertices = [ -5  2  0
              1 -2  3
              0  0 -1];

S = st_simplex(vertices);
reshape([S.constraints.coefficients],4,13)'-[zeros(13,3) [S.constraints.threshold]']

%T = euclidean_object('simplex',[vertices;1 1 1]);
%reshape([T.constraints.coefficients],4,9)'-[zeros(9,3) [T.constraints.threshold]']