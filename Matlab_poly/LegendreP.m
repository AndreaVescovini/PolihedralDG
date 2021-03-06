function [P, Pder] = LegendreP(x, N, int)
% function [P] = LegendreP(x, N, int)
% Evaluate (scaled) Legendre Polynomial and its derivative defined on the interval int
% at points x for order N.
%
% Author: Andrea Vescovini

% Turn points into row if needed.
xp = x;
dims = size(xp);
if (dims(2) == 1)
    xp = xp';
end

PL = zeros(N+1,length(xp));
Pder = zeros(1,length(xp));

hbinv = 2/(int(2)-int(1));
mb = (int(2)+int(1))/2;
xp = (xp - mb)*hbinv;

% Initial values L_0(x) and L_1(x)
PL(1,:) = 1.0;
PL(2,:) = xp;

% Forward recurrence using the symmetry of the recurrence.
for k = 2:N
  PL(k+1,:) = -(k-1)/k*PL(k-1,:) + (2*k-1)/k*xp.*PL(k,:);
end

for k = 0:1:floor((N-1)/2)
    Pder = Pder + PL(N-2*k,:)*(2*N-4*k-1);
end

P = (PL(N+1,:)')*sqrt(hbinv);
Pder = (Pder')*(hbinv)^1.5;
end
