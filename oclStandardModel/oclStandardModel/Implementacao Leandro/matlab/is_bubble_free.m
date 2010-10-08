function [result] = is_bubble_free(object,hyxels)

hyxels = hyxels';
n = size(hyxels,2);
m = object.dim;

multi_index = nchoosek(1:n,m+1);
for j=1:size(multi_index,1)
    samples = repmat({[-0.5 0.5]},1,m+1);
    [samples{:}] = ndgrid(samples{:});

    offset = cell(1,m+1);
    for i=1:m+1
        offset{i} = samples{i}(:);
    end
    offset = cell2mat(offset);

    proj_hyxels = unique(hyxels(:,multi_index(j,:)),'rows');
    
    bubbles = zeros(0,m+1);
    for i=1:2^(m+1)
        bubbles = cat(1,bubbles,proj_hyxels+repmat(offset(i,:),size(proj_hyxels,1),1));
    end
    bubbles = unique(bubbles,'rows');

    counter = zeros(size(bubbles,1),1);
    for i=1:2^(m+1)
        counter = counter+ismember(bubbles+repmat(offset(i,:),size(bubbles,1),1),proj_hyxels,'rows');
    end

    if any(floor(log(counter)/log(2))>m)
        result = false;
        return
    end
end

result = true;