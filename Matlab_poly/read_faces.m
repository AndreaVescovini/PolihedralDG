function [faces, faces_neig] = read_faces(mesh)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

% I make a preallocation by excess
tot_faces = mesh.Ntet*4;
% ext_faces = 12;
% tot_faces = (mesh.K*4+ext_faces)/2;

faces = zeros(tot_faces,3);
faces_sorted = zeros(tot_faces,3);
faces_neig = zeros(tot_faces,4); % number of E1 | local number of e in E1 | the same for E2
%elem_faces = zeros(mesh.Nt, 4); % faces that compose every element
ii = 1;
for ie = 1:mesh.Ntet
    for e = 1:4
        ff = mesh.E2V(ie, [1:4-e 6-e:4]);
        sff = sort(ff);
        [is, idx] = ismember(sff, faces_sorted, 'rows');
        if is == 0
            faces(ii,:) = ff;
            faces_sorted(ii,:) = sff;
            faces_neig(ii,1) = ie;
            faces_neig(ii,2) = e;
            %elem_faces(ie,e) = ii;
            ii = ii+1;
        else
            if (mesh.E2P(ie) == mesh.E2P(faces_neig(idx,1)))
                faces(idx,:) = [];
                faces_neig(idx,:) = [];
                faces_sorted(idx,:) = [];
                ii = ii-1;
                tot_faces = tot_faces-1;
            else
                faces_neig(idx,3) = ie;
                faces_neig(idx,4) = e;
                %elem_faces(ie,e) = idx;
            end
        end
    end
end

% shrink matrices
faces(ii:tot_faces, :) = [];
faces_neig(ii:tot_faces, :) = [];

end

