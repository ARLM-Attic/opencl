function [result] = test_discrete_points(constraints,dicrete_points)

assert(isstruct(constraints))
assert(isnumeric(dicrete_points) && ndims(dicrete_points)==2 && ~isempty(dicrete_points))

[n m] = size(dicrete_points);

result = true(1,true);
for i=1:numel(constraints)
    type = lower(constraints(i).type);
    switch type
        case 'eq'
            assert(n==numel(constraints(i).coefficients))
            result = and(result,all(dicrete_points==repmat(constraints(i).coefficients,1,m),1));

        case {'leq','le','leq le'}
            assert((n+1)==numel(constraints(i).coefficients))

            value = constraints(i).coefficients*[dicrete_points;ones(1,m)];
            threshold = constraints(i).threshold;

            switch type
                case 'leq'
                    result = and(result,value<=threshold);
                case 'le'
                    result = and(result,value<threshold);
                case 'leq le'
                    result = and(result,and(-threshold<=value,value<threshold));
            end

        otherwise
            error('Unknown constraint type ''%s''.',constraints(i).type)
    end
end