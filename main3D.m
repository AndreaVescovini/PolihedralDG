% Poisson problem with Dirichlet conditions
% -u'' = f    nel dominio
%    u = gd   sul bordo

% dati del problema
uex = @(x,y,z) exp(x.*y.*z);
f = @(x,y,z) -exp(x.*y.*z).*((x.*y).^2+(x.*z).^2+(y.*z).^2);
gd = uex;
N = 1; % degree
Np = (N+1)*(N+2)*(N+3)/6; %number of points for every element
Nfaces = 4; % number of faces for every element

% generate the mesh
[Nv, VX, VY, VZ, K, EToV] = MeshReaderGambit3D('meshes\cubeK5.neu');

% set the neighboors
[EToE, EToF] = tiConnect3D(EToV);

% compute dof
[x,y,z] = Nodes3D(N); [r,s,t] = xyztorst(x,y,z);
va = EToV(:,1)'; vb = EToV(:,2)'; vc = EToV(:,3)'; vd = EToV(:,4)';
x = 0.5*(-(1+r+s+t)*VX(va)+(1+r)*VX(vb)+(1+s)*VX(vc)+(1+t)*VX(vd));
y = 0.5*(-(1+r+s+t)*VY(va)+(1+r)*VY(vb)+(1+s)*VY(vc)+(1+t)*VY(vd));
z = 0.5*(-(1+r+s+t)*VZ(va)+(1+r)*VZ(vb)+(1+s)*VZ(vc)+(1+t)*VZ(vd));

% compute the jacobians
%%%%%% BISOGNA CONTROLLARE CHE SE N>1 A ME INTERESSANO SOLO I VERTICI,
%%%%%% DIPENDE DA COME CREO PRIMA r s t, CHE TRAMITE EToV MAPPANO x y z,
%%%%%% POTREI PER ESEMPIO METTERE I VERTICI SUBITO NEI PRIMI NODI.
[Jdet, Jcof] = jacobians(x,y,z,r,s,t); % del determinante dovro' poi prenderne il valore assoluto

% assemble the linear system
[A, b] = linsys(K, Np, Jdet, Jcof);

% solve the linear system
%u = A\b;
