 function [result] = st_simplex(vertices)

assert(isnumeric(vertices) && ndims(vertices)==2 && size(vertices,1)>=1 && size(vertices,2)>=1 && size(vertices,2)<=size(vertices,1)+1)

n = size(vertices,1);
m = size(vertices,2)-1;
vertices = [vertices;ones(1,m+1)]; % Set homogeneous coordinate to 1.

constraints = [];


for d=1:min(m+1,n)
    multi_indices = nchoosek(1:n,d);

    for j=1:size(multi_indices,1)
        
        % Project vertices onto the d-dimensional space defined by current
        % multi-index.,
        vertices_proj = vertices([multi_indices(j,:) n+1],:);

        % Compute matrix representation of the flat induced by the vertices
        % projected onto current d-dimensional space (i.e., the reduced
        % column echelon form of vertices_proj) and its dimensionality
        % (i.e., the rank of flat's matrix).
        [flat_proj flat_proj_rank] = induced_flat(vertices_proj);
        
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        % Compute constraints from induced flat if it is an hyperplane in
        % current d-dimensional space.
        if flat_proj_rank == d
            coefficients = half_space(n,d,[multi_indices(j,:) n+1],flat_proj);

            % Set standard orientation.
            if has_standard_orientation(coefficients)
                constraints = cat(1,constraints,constraint('leq le',coefficients,threshold(coefficients)));
            else
                constraints = cat(1,constraints,constraint('leq le',-coefficients,threshold(coefficients)));
            end
        
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        % Compute constraints from extrusion only if projected vertices
        % induce current d-dimensional space.
        elseif flat_proj_rank == d+1
          %%{ 
            % Compute the convex hull of projected vertices in order to get
            % the facets of the convex polytope defined by those points.
            % But first, remove duplicated entries.
            if d > 1
                % Create constraints from the extrusion of half-spaces bounded
                % by the facets of the convex polytope.
                unique_vertices_proj = unique(vertices_proj','rows')';
                facets = convhulln(unique_vertices_proj(1:d,:)');

                for f=1:size(facets,1)
                    coefficients = half_space(n,d,[multi_indices(j,:) n+1],unique_vertices_proj(:,facets(f,:)));

                    if all((coefficients*vertices)<=0,2)
                        constraints = cat(1,constraints,half_space_constraints(coefficients));
                    else
                        constraints = cat(1,constraints,half_space_constraints(-coefficients));
                    end
                end
            else
                % Trivial convex hull.
                range = [min(vertices_proj(1,:),[],2) max(vertices_proj(1,:),[],2)];

                % Create constraints from the extrusion of such range.
                coefficients = zeros(1,n+1);
                coefficients(multi_indices(j,:)) = 1;
                coefficients(n+1) = -(range(1)+range(2))/2;

                bound = threshold(coefficients)+coefficients(n+1)+range(2);
                constraints = cat(1,constraints,constraint('leq le',coefficients,bound));
            end
            %%}
        end
    end
end

result = struct('type','simplex','dim',size(vertices,2)-1,'constraints',constraints);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [result] = half_space_constraints(coefficients)

if has_standard_orientation(coefficients)
    result = constraint('le',coefficients,threshold(coefficients));
else
    result = constraint('leq',coefficients,threshold(coefficients));
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [coefficients] = half_space(n,d,basis,factors)
% Compute the coefficients of the hyperplane by using Laplace expansion.
coefficients = zeros(1,n+1);
for i=1:d+1
    %factors([1:i-1 i+1:d+1],1:d);
    coefficients(basis(i)) = (-1)^((d+1)+i)*det(factors([1:i-1 i+1:d+1],1:d));
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [result] = has_standard_orientation(coefficients)

result = coefficients(find(coefficients(1:end-1),1))>0;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [result] = threshold(coefficients)

result = sum(abs(coefficients(1:end-1)),2)/2;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [result rank] = induced_flat(vertices)

[rref_matrix jb] = rref(vertices');
result = rref_matrix(any(rref_matrix~=0,2),:)';
rank = length(jb);