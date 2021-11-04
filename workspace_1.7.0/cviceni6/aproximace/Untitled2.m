ntc=csvread('ntc.csv');
t=ntc(:,1);
ad_t=[]

for i=1:length(ntc(:,2))
   
ad=1024*(ntc(i,2)/(10+ntc(i,2)));
ad_t=[ad_t ad];
end



plot(ad_t,t)
hold on
%axis([0 200 -50 150]
xlabel('ADC value')
ylabel('Temperature[°C]')
ad_t=transpose(ad_t);
p = polyfit(ad_t, t, 10);
ad2 = 0:1023;
t2 = round(polyval(p, ad2), 1);
plot(ad2, t2, 'r');
dlmwrite('data.dlm', t2*10, ',');
t3 = t2.*10