close all
clear
clc

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% euclidean_space_bounds = [-10 10 -10 10];
% 
% objects = euclidean_object('half-space',[3 -7],0);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

euclidean_space_bounds = [-10 10 -10 10];

% p1 = [3;-9;1];
% p2 = [0; 0;1];

% p1 = [3;-9;1];
% p2 = [0; 1;0];

% p1 = [ 6; 7; 1];
% p2 = [-9;-8; 1];

% p1 = [ 6; 8; 1];
% p2 = [-9;-8; 1];

% p1 = [ 6; 3; 1];
% p2 = [-9;-6; 1];

p1 = [ 7; 3; 1];
p2 = [-8;-6; 1];

objects = [...
           euclidean_object('flat',[p1 p2])
%            euclidean_object('flat',p1)
%            euclidean_object('flat',p2)
          ];

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% euclidean_space_bounds = [-10 10 -10 10 -10 10];
% 
% % p1 = [0;0;0;1];
% % p2 = [6;7;3;1];
% 
% % p1 = [0;0;0;1];
% % p2 = [9;1;1;1];
% 
% % p1 = [ 6; 7; 3;1];
% % p2 = [-9;-8;-6;1];
% 
% % p1 = [0;0;0;1];
% % p2 = [1;0;0;0];
% 
% % p1 = [0;0;0;1];
% % p2 = [1;2;0;0];
% 
% p1 = [4;4;5;1];
% p2 = [3;4;5;1];
% 
% objects = [...
%            euclidean_object('flat',[p1 p2])
%            euclidean_object('flat',p1)
%            euclidean_object('flat',p2)
%           ];

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% euclidean_space_bounds = [3 4 4 5 1 2];
% p1 = [ 6; 7; 3;1];
% p2 = [-9;-8;-6;1];
% 
% % euclidean_space_bounds = [3 4 4 5];
% % p1 = [ 6; 7;1];
% % p2 = [-9;-8;1];
% 
% % euclidean_space_bounds = [3 4 1 2];
% % p1 = [ 6; 3;1];
% % p2 = [-9;-6;1];
% 
% % euclidean_space_bounds = [4 5 1 2];
% % p1 = [ 7; 3;1];
% % p2 = [-8;-6;1];
% 
% objects = euclidean_object('flat',[p1 p2]);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% euclidean_space_bounds = [-10 10 -10 10 -10 10 -10 10 -10 10];
% 
% p  = [0;1;2;3;4;1];
% v1 = [1;2;3;4;5;0];
% v2 = [3;3;6;8;10;0];
% objects = euclidean_object('flat',[p v1 v2]);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% euclidean_space_bounds = [-10 10 -10 10 -10 10 -10 10];
% 
% p1 = [-9; 9; 0; 0;1];
% p2 = [ 9; 9; 0; 0;1];
% p3 = [-9;-9; 9; 0;1];
% 
% % objects = [...
% %            euclidean_object('flat',[p1 p2 p3])
% %            euclidean_object('flat',[p1 p2])
% %            euclidean_object('flat',[p1 p3])
% %            euclidean_object('flat',[p2 p3])
% %            euclidean_object('flat',p1)
% %            euclidean_object('flat',p2)
% %            euclidean_object('flat',p3)
% %           ];
% 
% objects = [...
%            euclidean_object('flat',[p1 p2])
%            euclidean_object('flat',[p1 p3])
%            euclidean_object('flat',[p2 p3])
%            euclidean_object('flat',p1)
%            euclidean_object('flat',p2)
%            euclidean_object('flat',p3)
%           ];

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% euclidean_space_bounds = [-10 10 -10 10 -10 10 -10 10 -10 10];
% 
% p0 = [-9;-9;-9; 0;0;1];
% p1 = [-9;-9; 9; 0;0;1];
% p2 = [-9; 9; 9; 0;0;1];
% p3 = [-9;-9;-9; 9;0;1];
% 
% objects = [...
%            euclidean_object('flat',[p0 p1 p2 p3])
%            euclidean_object('flat',[p0 p1 p2])
%            euclidean_object('flat',[p0 p1 p3])
%            euclidean_object('flat',[p0 p2 p3])
%            euclidean_object('flat',[p1 p2 p3])
%            euclidean_object('flat',[p0 p1])
%            euclidean_object('flat',[p0 p2])
%            euclidean_object('flat',[p0 p3])
%            euclidean_object('flat',[p1 p2])
%            euclidean_object('flat',[p1 p3])
%            euclidean_object('flat',[p2 p3])
%            euclidean_object('flat',p0)
%            euclidean_object('flat',p1)
%            euclidean_object('flat',p2)
%            euclidean_object('flat',p3)
%           ];

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% % euclidean_space_bounds = [-10 10 -10 10];
% % 
% % % p0 = [-5;-3;1];
% % % p1 = [-5; 7;1];
% % % p2 = [ 8;-3;1];
% % 
% % % p0 = [-5;-3;1];
% % % p1 = [-3; 7;1];
% % % p2 = [ 8;-3;1];
% % 
% % p0 = [0;0;1];
% % p1 = [9;1;1];
% % p2 = [3;8;1];
% % 
% % objects = [...
% %            euclidean_object('simplex',[p0 p1 p2])
% %            euclidean_object('simplex',[p0 p1])
% %            euclidean_object('simplex',[p0 p2])
% %            euclidean_object('simplex',[p1 p2])
% %            euclidean_object('simplex',p0)
% %            euclidean_object('simplex',p1)
% %            euclidean_object('simplex',p2)
% %           ];

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% euclidean_space_bounds = [-10 10 -10 10 -10 10];
% 
% p0 = [ 0; 0; 0;1];
% p1 = [ 3; 8; 4;1];
% p2 = [ 9; 1; 1;1];
% p3 = [-3;-5;-6;1];
% 
% objects = [...
%            euclidean_object('simplex',[p0 p1 p2])
%            euclidean_object('simplex',[p0 p1])
%            euclidean_object('simplex',[p0 p2])
%            euclidean_object('simplex',[p1 p2])
%            euclidean_object('simplex',p0)
%            euclidean_object('simplex',p1)
%            euclidean_object('simplex',p2)
%            ];

% objects = [...
%            euclidean_object('simplex',[p0 p1])
%            euclidean_object('simplex',[p0 p2])
%            euclidean_object('simplex',[p1 p2])
%            euclidean_object('simplex',p0)
%            euclidean_object('simplex',p1)
%            euclidean_object('simplex',p2)
%            ];
% 
% objects = [...
%            euclidean_object('simplex',[p0 p2 p3])
%            euclidean_object('simplex',[p0 p2])
%            euclidean_object('simplex',[p0 p3])
%            euclidean_object('simplex',[p2 p3])
%            euclidean_object('simplex',p0)
%            euclidean_object('simplex',p2)
%            euclidean_object('simplex',p3)
%            ];

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% euclidean_space_bounds = [-10 10 -10 10 -10 10];
% 
% p0 = [-5;-5;-5;1];
% p1 = [-5;-5; 5;1];
% p2 = [-5; 5; 5;1];
% p3 = [ 5;-5;-5;1];
% 
% % p0 = [-5;-5;-5;1];
% % p1 = [-7;-5; 5;1];
% % p2 = [-5; 5; 5;1];
% % p3 = [ 5;-5;-5;1];
% 
% % p0 = [-5;-8;-5;1];
% % p1 = [-7;-5; 4;1];
% % p2 = [-9; 3; 2;1];
% % p3 = [ 3;-2;-8;1];
% 
% objects = [...
%            euclidean_object('simplex',[p0 p1 p2 p3])
%            euclidean_object('simplex',[p0 p1 p2])
%            euclidean_object('simplex',[p0 p1 p3])
%            euclidean_object('simplex',[p0 p2 p3])
%            euclidean_object('simplex',[p1 p2 p3])
%            euclidean_object('simplex',[p0 p1])
%            euclidean_object('simplex',[p0 p2])
%            euclidean_object('simplex',[p0 p3])
%            euclidean_object('simplex',[p1 p2])
%            euclidean_object('simplex',[p1 p3])
%            euclidean_object('simplex',[p2 p3])
%            euclidean_object('simplex',p0)
%            euclidean_object('simplex',p1)
%            euclidean_object('simplex',p2)
%            euclidean_object('simplex',p3)
%            ];

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% euclidean_space_bounds = [-10 10 -10 10 -10 10 -10 10 -10 10];
% 
% p0 = [-9;-9;-9; 0;0;1];
% p1 = [-9;-9; 9; 0;0;1];
% p2 = [-9; 9; 9; 0;0;1];
% p3 = [-9;-9;-9; 9;0;1];
% 
% objects = [...
%            euclidean_object('simplex',[p0 p1 p2 p3])
%            euclidean_object('simplex',[p0 p1 p2])
%            euclidean_object('simplex',[p0 p1 p3])
%            euclidean_object('simplex',[p0 p2 p3])
%            euclidean_object('simplex',[p1 p2 p3])
%            euclidean_object('simplex',[p0 p1])
%            euclidean_object('simplex',[p0 p2])
%            euclidean_object('simplex',[p0 p3])
%            euclidean_object('simplex',[p1 p2])
%            euclidean_object('simplex',[p1 p3])
%            euclidean_object('simplex',[p2 p3])
%            euclidean_object('simplex',p0)
%            euclidean_object('simplex',p1)
%            euclidean_object('simplex',p2)
%            euclidean_object('simplex',p3)
%           ];

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

{objects(1).constraints.type}', [cell2mat({objects(1).constraints.coefficients}') cell2mat({objects(1).constraints.threshold}')]

% Get the number of dimensions of the Euclidean space.
n = numel(euclidean_space_bounds)/2;

% Declare all discrete points we need.
discrete_space_bounds = ceil(euclidean_space_bounds-1/2);
discrete_samples = cell(n,1);
for i=1:n
    discrete_samples{i} = discrete_space_bounds((i-1)*2+1):discrete_space_bounds((i-1)*2+2);
end
[discrete_samples{:}] = ndgrid(discrete_samples{:});

all_dicrete_points = zeros(n,numel(discrete_samples{1}));
for i=1:n
    all_dicrete_points(i,:) = discrete_samples{i}(:);
end

% Compute hyxels.
hyxels = cell(size(objects));
for o=1:numel(objects)
    hyxels{o} = all_dicrete_points(:,test_discrete_points(objects(o).constraints,all_dicrete_points));

    if is_bubble_free(objects(o),hyxels{o})
        disp(sprintf('Object %d is bubble free.',o))
    else
        disp(sprintf('Object %d has bubbles <------------------------ Warning',o))
    end
end

% Display results.
if n == 2 || n == 3
    colors = hsv(numel(objects));
    markers = {'.','s','o','*'};
    
    figure
    axis(euclidean_space_bounds)
    axis square
    box on
    xlabel('e_{1}')
    ylabel('e_{2}')
    zlabel('e_{3}')
    hold on

    for o=1:numel(objects)
        coords = mat2cell(hyxels{o},ones(n,1),size(hyxels{o},2));
        line(coords{:},'LineStyle','none','Marker',markers{objects(o).dim+1},'Color',colors(o,:))
    end

    hold off
end