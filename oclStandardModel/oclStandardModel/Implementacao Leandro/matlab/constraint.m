function [result] = constraint(type,varargin)

switch lower(type)
    case 'eq'
        assert(numel(varargin)==1)
        result = struct('type','eq','coefficients',varargin{1},'threshold',[]);
    case {'leq','le','leq le'}
        assert(numel(varargin)==2)
        result = struct('type',lower(type),'coefficients',varargin{1},'threshold',varargin{2});
    otherwise
        error('Unknown constraint type ''%s''.',type)
end