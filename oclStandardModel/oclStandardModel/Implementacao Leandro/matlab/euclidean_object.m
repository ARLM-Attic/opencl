function [result] = euclidean_object(type,varargin)

switch lower(type)
    case 'point'
        assert(numel(varargin)==1)

        coordinates = varargin{1};
        assert(isnumeric(coordinates) && isvector(coordinates) && size(coordinates,1)>1);
        
        result = struct('type','point','dim',0,'constraints',point_constraints(coordinates));
        
    case 'half-space'
        assert(numel(varargin)==2)

        coefficients = varargin{1};
        assert(isnumeric(coefficients) && isvector(coefficients) && size(coefficients,1)==1 && size(coefficients,2)>=1)
        
        constant = varargin{2};
        assert(isnumeric(constant) && isscalar(constant))

        result = struct('type','half-space','dim',numel(coefficients),'constraints',half_space_constraints(coefficients,constant));
        
    case 'flat'
        assert(numel(varargin)==1)
        
        factors = varargin{1};
        assert(isnumeric(factors) && ndims(factors)==2 && size(factors,1)>1 && size(factors,2)>=1)

        result = struct('type','flat','dim',size(factors,2)-1,'constraints',flat_constraints(factors));

    case 'simplex'
        assert(numel(varargin)==1)
        
        vertices = varargin{1};
        assert(isnumeric(vertices) && ndims(vertices)==2 && size(vertices,1)>1 && size(vertices,2)>=1)

        result = struct('type','simplex','dim',size(vertices,2)-1,'constraints',simplex_constraints(vertices));
        
    otherwise
        error('Unknown Euclidean object type ''%s''.',type)
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [result] = point_constraints(coordinates)

n = numel(coordinates)-1;
result = constraint('eq',ceil(coordinates(1:n)./coordinates(n+1)-1/2));

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [result] = half_space_constraints(coefficients,constant)

if has_standard_orientation(coefficients)
    result = constraint('le',[coefficients -constant],threshold(coefficients));
else
    result = constraint('leq',[coefficients -constant],threshold(coefficients));
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [result] = flat_constraints(factors)

n = size(factors,1)-1;
k = size(factors,2)-1;

% A 0-flats is a point.
if k == 0
    result = point_constraints(factors);

% Some k-flat, where 0 < k < n-1.
elseif (0 < k) && (k < n-1)
    result = k_flat_constraints(n,k,factors);

% A (n-1)-flat is an hyperplane.
elseif k == n-1
    result = hyperplane_constraints(n,factors);
    
% Invalid flat.
else
    error('Invalid flat.')
    
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [result] = hyperplane_constraints(n,factors)

% Compute the coefficients of the hyperplane.
[coefficients constant] = half_space(n,n,1:n,factors);

% Setup constraints.
if has_standard_orientation(coefficients)
    result = constraint('leq le',[coefficients -constant],threshold(coefficients));
else
    result = constraint('leq le',[-coefficients constant],threshold(coefficients));
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [result] = k_flat_constraints(n,k,factors)

result = [];

for m=k+1:-1:1
    multi_indices = nchoosek(1:n,m);

    for i=1:size(multi_indices,1)
        % Project factors into some m-dimensional space.
        proj = project(multi_indices(i,:),factors);

        % Create constraints only if projected factors expands a non-axis
        % aligned hyperplane in current m-dimensional space. The
        % constraints for other cases will be created by some other step of
        % the process. Compute factors for projected subspace using reduced
        % column echelon form.
        if rank(proj) == m
            if m == k+1
                factors_proj = proj;
            else
                factors_proj = rref(proj')';
                factors_proj = factors_proj(:,any(factors_proj~=0,1));
            end

            [coefficients constant] = half_space(n,m,multi_indices(i,:),factors_proj);

            % Ignore the hyperplane if it is parallel to some basis
            % vector. The constraints for such hyperplane will be
            % defined at some other step of the process.
            if nnz(coefficients) == m
                if has_standard_orientation(coefficients)
                    result = cat(1,result,constraint('leq le',[coefficients -constant],threshold(coefficients)));
                else
                    result = cat(1,result,constraint('leq le',[-coefficients constant],threshold(coefficients)));
                end
            end
        end
    end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [result] = simplex_constraints(vertices)

n = size(vertices,1)-1;
k = size(vertices,2)-1;

% The simplex is a point.
if k == 0
    result = point_constraints(vertices);

% The simplex induces an k-D subspace.
elseif (0 < k) && (k <= n-1)
    result = k_simplex_constraints(n,k,vertices);

% The simplex has n+1 vertices.
elseif k == n
    result = n_simplex_constraints(n,vertices);
    
% Invalid simplex.
else
    error('Invalid simplex.')
    
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [result] = k_simplex_constraints(n,k,vertices)

% Compute constraints from induced flat.
result = flat_constraints(vertices);

% Compute constraints from extrusion of the given k-simplex from
% m-subspaces expanded by vector basis, where 1 <= m <= k.
for m=k:-1:1
    result = cat(1,result,extrusion_constraints(n,m,vertices));
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [result] = n_simplex_constraints(n,vertices) % This method is a specialization of k_simplex_constraints(n,k,vertices).

result = [];

% Compute half-spaces from the faces of the given n-simplex. This is a
% simplicication of extrusion_constraints(n,n,vertices).
for i=0:n
    [coefficients constant] = half_space(n,n,1:n,vertices(:,[1:(i+1)-1 (i+1)+1:n+1]));
    
    % Ignore the half-space if it is parallel to some basis vector. The
    % constraints for such half-space will be defined by the extrusion of
    % the given n-simplex.
    if nnz(coefficients) == n
        if ([coefficients -constant]*vertices(:,i+1)) <= 0
            result = cat(1,result,half_space_constraints( coefficients, constant));
        else
            result = cat(1,result,half_space_constraints(-coefficients,-constant));
        end
    end
end

% Compute constraints from extrusion of given n-simplex from
% m-subspaces expanded by vector basis, where 1 <= m <= n-1.
for m=n-1:-1:1
    result = cat(1,result,extrusion_constraints(n,m,vertices));
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [result] = extrusion_constraints(dims,n,vertices)

result = [];

if n > 1
    multi_indices = nchoosek(1:dims,n);

    for i=1:size(multi_indices,1)
        % Project the vertices of the simplex into some n-dimensional space.
        proj = project(multi_indices(i,:),vertices);

        % Create constraints only if projected vertices induces current
        % n-dimensional space. The constraints for other cases will be
        % created by some other step of the process.
        if rank(proj) == n+1
            % Compute the convex hull of projected points in order to get
            % the facets of the convex polytope defined by those points.
            % But first, remove duplicated entries.
            points_proj = proj(1:n,:)./repmat(proj(n+1,:),n,1); %TODO Matei as coordenadas homogêneas.
            points_proj = unique(points_proj','rows')';
            vertices_proj = [points_proj;ones(1,size(points_proj,2))];

            facets = convhulln(points_proj');
            
            % Create constraints from the extrusion of half-spaces bounded
            % by the facets of the convex polytope.
            for j=1:size(facets,1)
                [coefficients constant] = half_space(dims,n,multi_indices(i,:),vertices_proj(:,facets(j,:)));

                % Ignore the half-space if it is parallel to some basis
                % vector. The constraints for such half-space will be
                % defined at some other step of the process.
                if nnz(coefficients) == n
                    if all(([coefficients -constant]*vertices)<=0,2)
                        result = cat(1,result,half_space_constraints( coefficients, constant));
                    else
                        result = cat(1,result,half_space_constraints(-coefficients,-constant));
                    end
                end
            end
        end
    end
else
    for i=1:dims
        % Projected vertices define a range of values in some 1-D space.
        proj = project(i,vertices);

        % Create constraints only if projected vertices induces current
        % n-dimensional space. The constraints for other cases will be
        % created by some other step of the process.
        if rank(proj) == n+1
            points_proj = proj(1,:)./proj(2,:);
            range = [min(points_proj,[],2) max(points_proj,[],2)];

            % Create constraints from the extrusion of such range.
            coefficients = zeros(1,dims);
            coefficients(i) = 1;

            constant = (range(1)+range(2))/2;
            assert(sum(range-constant)==0)

            bound = threshold(coefficients)+range(2)-constant;

            result = cat(1,result,constraint('leq le',[coefficients -constant],bound));
        end
    end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [coefficients constant] = half_space(dims,n,basis,factors)

% Compute the coefficients of the hyperplane by using Laplace expansion.
row_major = factors';
coefficients = zeros(1,dims);
for i=1:n
    coefficients(basis(i)) = (-1)^((n+1)+i)*det(row_major(1:n,[1:i-1 i+1:n+1]));
end
% The sign of the constant term is always the negation of the determinant.
constant = -det(row_major(1:n,[1:(n+1)-1 (n+1)+1:n+1]));

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [result] = has_standard_orientation(coefficients)

result = coefficients(find(coefficients,1))>0;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [result] = threshold(coefficients)

result = sum(abs(coefficients),2)/2;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [result] = project(multi_index,factors)

n = size(factors,1)-1;
result = factors([multi_index n+1],:);