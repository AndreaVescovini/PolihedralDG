% 3D Poisson problem with Dirichlet conditions
% -lapl(u) = f    in the domain
%    u = gd   on the boundary

% uex = @(p) exp(p(1,:).*p(2,:).*p(3,:));
% f = @(p) -uex(p).*((p(1,:).*p(2,:)).^2+(p(1,:).*p(3,:)).^2+(p(2,:).*p(3,:)).^2);
% uex_grad = @(p) [uex(p).*p(2,:).*p(3,:); uex(p).*p(1,:).*p(3,:); uex(p).*p(1,:).*p(2,:)];

uex = @(p) p(1,:).^3+10*p(2,:).*p(3,:).^2;
f = @(p) -20*p(2,:)-6*p(1,:);
uex_grad = @(p) [3*p(1,:).^2; 10*p(3,:).^2; 20*p(2,:).*p(3,:)];

% uex = @(p) p(1,:).*p(2,:);
% f = @(p) 0;
% uex_grad = @(p) [p(2,:); p(1,:); 0];
 
% uex = @(p) p(1,:);
% f = @(p) 0;
% uex_grad = @(p) [1; 0; 0];

% uex = @(p) (p(1,:)).^1.5;
% f = @(p) -0.75./sqrt(p(1,:));
% uex_grad = @(p) [1.5*sqrt(p(1,:)); 0; 0];

% problem data
gd = uex;
N = 2; % degree of finite finite elements
sigma = 10; % penalty coefficient
epsilon = -1; % method

% generate the mesh and connectivity matrices
% mesh = MeshReader3Dpoly('../meshes/cube_str48p.mesh');
% mesh = MeshReader3D('../meshes/cube_str6.mesh');
mesh = MeshReaderGambit3D('../meshes/cubeK268.neu');

% build the linear system and solve the problem
[u, x, y, z] = linsys(mesh, f, gd, N, sigma, epsilon);

% compute errors
[err_L2, err_H10] = errors(uex, uex_grad, u, x, y, z, N);