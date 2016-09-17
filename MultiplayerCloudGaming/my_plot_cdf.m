%% eligible RDatacenter count cdf (basic problem) seperate session sizes (10 and 50 only)
L_G = [75 150];
L_R = [50 100];

legend_name = repmat('', 2, 1);
line_style = {'-', ':'};

ss = get(0, 'ScreenSize');
set(gcf, 'Position', [ss(1) ss(2) ss(3) ss(4)/3]);
pos = 0;
for size = [10 50]
    pos = pos + 1;
    subplot(1, 2, pos);
    for i = 1:2
        data = importdata(sprintf('%d_%d_%d_eligibleRDatacenterCount.csv', L_G(i), L_R(i), size));
        p_h = cdfplot(data);    
        hold on;
        set(p_h, 'LineStyle', line_style{i}, 'LineWidth', 3, 'Color', 'k'); 
        legend_name{i} = sprintf('(L_G = %d, L_R = %d)', L_G(i)*2, L_R(i)*2);
    end
    set(gca, 'XLim', [1 10]);
    set(gca, 'fontsize', 14);
    lh = legend(legend_name, 'Orientation', 'vertical', 'Location', 'southeast');
    set(lh, 'FontSize', 14);
    xlabel('Number of eligible datacenters per client', 'FontSize', 18);
    ylabel('Cumulative distribution function', 'FontSize', 16);   
    title(sprintf('|C| = %d', size));
    grid on;
    hold off;
end
export_fig cdf_rs_basic.pdf -transparent

%% eligible GDatacenter count cdf (general problem) seperate session sizes (10 and 50 only)
L_G = [75 150];
L_R = [50 100];

legend_name = repmat('', 2, 1);
line_style = {'-', ':'};

ss = get(0, 'ScreenSize');
set(gcf, 'Position', [ss(1) ss(2) ss(3) ss(4)/3]);
pos = 0;
for size = [10 50]
    pos = pos + 1;
    subplot(1, 2, pos);
    for i = 1:2
        data = importdata(sprintf('%d_%d_%d_eligibleGDatacenterCount.csv', L_G(i), L_R(i), size));
        p_h = cdfplot(data);    
        hold on;
        set(p_h, 'LineStyle', line_style{i}, 'LineWidth', 3, 'Color', 'k'); 
        legend_name{i} = sprintf('(L_G = %d, L_R = %d)', L_G(i)*2, L_R(i)*2);
    end
    set(gca, 'XLim', [1 13]);
    set(gca, 'XTick', [0 1 2 3 4 5 6 7 8 9 10 11 12 13]);    
    set(gca, 'fontsize', 14);
    lh = legend(legend_name, 'Orientation', 'vertical', 'Location', 'southeast');
    set(lh, 'FontSize', 14);
    xlabel('Number of eligible datacenters for placing the G-server', 'FontSize', 18);
    ylabel('Cumulative distribution function', 'FontSize', 16);
    title(sprintf('|C| = %d', size));
    grid on;   
    hold off;
end
export_fig cdf_gs_general.pdf -transparent
%% eligible RDatacenter count cdf (basic problem) all session sizes
L_G = [75 150];
L_R = [50 100];
legend_name = repmat('', 2, 1);
line_style = {'-', ':'};

for i = 1:2
    data = importdata(sprintf('%d_%d_eligibleRDatacenterCount.csv', L_G(i), L_R(i))); 
    p_h = cdfplot(data);    
    hold on;
    set(p_h, 'LineStyle', line_style{i}, 'LineWidth', 3, 'Color', 'k'); 
    legend_name{i} = sprintf('(L_G = %d, L_R = %d)', L_G(i)*2, L_R(i)*2);
end

set(gca, 'XLim', [1 10]);
%set(gca, 'XTick', [0 1 2 3 4 5 6 7 8 9 10 11 12 13]);
%set(gca, 'YTick', [0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1]);
set(gca, 'fontsize', 14);
lh = legend(legend_name, 'Orientation', 'vertical', 'Location', 'southeast');
set(lh, 'FontSize', 14);
xlabel('Number of eligible datacenters per client (basic problem)', 'FontSize', 16);
ylabel('Cumulative Dist. Function', 'FontSize', 16);
title('');

pbaspect([2.5 1 1]);
grid on;
file_name = 'cdf_eligibleRDatacenterCount';
%savefig(file_name);
print(file_name, '-dmeta');

%% eligible GDatacenter count cdf (general problem) all session sizes
L_G = [75 150];
L_R = [50 100];
legend_name = repmat('', 2, 1);
line_style = {'-', ':'};

for i = 1:2
    data = importdata(sprintf('%d_%d_eligibleGDatacenterCount.csv', L_G(i), L_R(i))); 
    p_h = cdfplot(data);    
    hold on;
    set(p_h, 'LineStyle', line_style{i}, 'LineWidth', 3, 'Color', 'k'); 
    legend_name{i} = sprintf('(L_G = %d, L_R = %d)', L_G(i)*2, L_R(i)*2);
end

set(gca, 'XLim', [1 13]);
set(gca, 'XTick', [0 1 2 3 4 5 6 7 8 9 10 11 12 13]);
%set(gca, 'YTick', [0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1]);
set(gca, 'fontsize', 14);
lh = legend(legend_name, 'Orientation', 'vertical', 'Location', 'southeast');
set(lh, 'FontSize', 14);
xlabel('Number of eligible datacenters for G-server placement (general problem)', 'FontSize', 16);
ylabel('Culmulative Dist. Function', 'FontSize', 16);
title('');

pbaspect([2.5 1 1]);
grid on;
file_name = 'cdf_eligibleGDatacenterCount';
%savefig(file_name);
print(file_name, '-dmeta');

%% GDatacenter (basic problem)
L_G = [75 150];
L_R = [50 100];
size = [10 50];
y_max = 20;
for j = 1:2
    for i = 1:2
        file_name = sprintf('%d_%d_GDatacenterID.csv', L_G(i), L_R(i), size(j));
        data = importdata(file_name);
        data = data + 1;
        histogram(data);
        pbaspect([2 1 1]);
        set(gca, 'fontsize', 16);
        set(gca, 'XLim', [0 14]);
        set(gca, 'XTick', [1 2 3 4 5 6 7 8 9 10 11 12 13]);
        set(gca, 'YLim', [0 y_max]);
        xlabel('Index of the datacenter for running the G-server: d_g', 'FontSize', 20);
        ylabel('Frequency', 'FontSize', 20);
        title(sprintf('(L_G = %d, L_R = %d); \t (|C| = %d)', L_G(i)*2, L_R(i)*2, size(j)));
        grid on;
        savefig(file_name);
        print(file_name, '-dmeta');
    end
end

%% GDatacenter (general problem) LCP
y_max = [
    150 350;
    150 200];

L_G = [75 150];
L_R = [50 100];
size = [10 50];
for j = 1:2
    for i = 1:2
        figure;
        file_name = sprintf('%d_%d_finalGDatacenterID.csv', L_G(i), L_R(i), size(j));
        data = importdata(file_name);
        data = data + 1;
        data = data(:, 6); % LSP only        
        histogram(data);
        pbaspect([2 1 1]);
        set(gca, 'fontsize', 16);
        set(gca, 'XLim', [0 14]);
        set(gca, 'XTick', [1 2 3 4 5 6 7 8 9 10 11 12 13]);
        set(gca, 'YLim', [0 y_max(j, i)]);
        xlabel('Index of the datacenter for running the G-server: d_g', 'FontSize', 20);
        ylabel('Frequency', 'FontSize', 20);
        title(sprintf('(L_G = %d, L_R = %d); \t (|C| = %d); \t LCP', L_G(i)*2, L_R(i)*2, size(j)));
        grid on;
        savefig(strcat(file_name, '_LCP'));
        print(strcat(file_name, '_LCP'), '-dmeta');
    end
end

%% GDatacenter (general problem) LCW
y_max = [
    150 350;
    150 200];

L_G = [75 150];
L_R = [50 100];
size = [10 50];
for j = 1:2
    for i = 1:2
        figure;
        file_name = sprintf('%d_%d_finalGDatacenterID.csv', L_G(i), L_R(i), size(j));
        data = importdata(file_name);
        data = data + 1;
        data = data(:, 7); % LCW only        
        histogram(data);
        pbaspect([2 1 1]);
        set(gca, 'fontsize', 16);
        set(gca, 'XLim', [0 14]);
        set(gca, 'XTick', [1 2 3 4 5 6 7 8 9 10 11 12 13]);
        set(gca, 'YLim', [0 y_max(j, i)]);
        xlabel('Index of the datacenter for running the G-server: d_g', 'FontSize', 20);
        ylabel('Frequency', 'FontSize', 20);
        title(sprintf('(L_G = %d, L_R = %d); \t (|C| = %d); \t LCW', L_G(i)*2, L_R(i)*2, size(j)));
        grid on;
        savefig(strcat(file_name, '_LCW'));
        print(strcat(file_name, '_LCW'), '-dmeta');
    end
end

%% GDatacenter (general problem) LAC
y_max = [
    150 350;
    150 200];

L_G = [75 150];
L_R = [50 100];
size = [10 50];
for j = 1:2
    for i = 1:2
        figure;
        file_name = sprintf('%d_%d_finalGDatacenterID.csv', L_G(i), L_R(i), size(j));
        data = importdata(file_name);
        data = data + 1;
        data = data(:, 8); % LAC only        
        histogram(data);
        pbaspect([2 1 1]);
        set(gca, 'fontsize', 16);
        set(gca, 'XLim', [0 14]);
        set(gca, 'XTick', [1 2 3 4 5 6 7 8 9 10 11 12 13]);
        set(gca, 'YLim', [0 y_max(j, i)]);
        xlabel('Index of the datacenter for running the G-server: d_g', 'FontSize', 20);
        ylabel('Frequency', 'FontSize', 20);
        title(sprintf('(L_G = %d, L_R = %d); \t (|C| = %d); \t LAC', L_G(i)*2, L_R(i)*2, size(j)));
        grid on;
        savefig(strcat(file_name, '_LAC'));
        print(strcat(file_name, '_LAC'), '-dmeta');
    end
end

%% server count per dc LCP
L_G = [75 150];
L_R = [50 100];
size = [10 50];
for j = 1:2
    for i = 1:2
        figure;
        file_name = sprintf('%d_%d_serverCountPerDC.csv', L_G(i), L_R(i), size(j));
        data = importdata(file_name);
        data = data(1, :)./100; % LCP
        bar(data);
        pbaspect([2 1 1]);
        set(gca, 'fontsize', 16);
        set(gca, 'XLim', [0 14]);
        set(gca, 'XTick', [1 2 3 4 5 6 7 8 9 10 11 12 13]);        
        xlabel('Index of datacenter (1 to 13)', 'FontSize', 20);
        ylabel('Number of opened servers', 'FontSize', 20);
        title(sprintf('(L_G = %d, L_R = %d); \t (|C| = %d); \t (k = 8) \t LCP', L_G(i)*2, L_R(i)*2, size(j)));
        grid on;
        savefig(strcat(file_name, '_LCP'));
        print(strcat(file_name, '_LCP'), '-dmeta');
    end
end

%% server count per dc LCW
L_G = [75 150];
L_R = [50 100];
size = [10 50];
for j = 1:2
    for i = 1:2
        figure;
        file_name = sprintf('%d_%d_serverCountPerDC.csv', L_G(i), L_R(i), size(j));
        data = importdata(file_name);
        data = data(2, :)./100; % LCW
        bar(data);
        pbaspect([2 1 1]);
        set(gca, 'fontsize', 16);
        set(gca, 'XLim', [0 14]);
        set(gca, 'XTick', [1 2 3 4 5 6 7 8 9 10 11 12 13]);        
        xlabel('Index of datacenter (1 to 13)', 'FontSize', 20);
        ylabel('Number of opened servers', 'FontSize', 20);
        title(sprintf('(L_G = %d, L_R = %d); \t (|C| = %d); \t (k = 8) \t LCW', L_G(i)*2, L_R(i)*2, size(j)));
        grid on;
        savefig(strcat(file_name, '_LCW'));
        print(strcat(file_name, '_LCW'), '-dmeta');
    end
end

%% server count per dc LAC
L_G = [75 150];
L_R = [50 100];
size = [10 50];
for j = 1:2
    for i = 1:2
        figure;
        file_name = sprintf('%d_%d_serverCountPerDC.csv', L_G(i), L_R(i), size(j));
        data = importdata(file_name);
        data = data(3, :)./100; % LAC
        bar(data);
        pbaspect([2 1 1]);
        set(gca, 'fontsize', 16);
        set(gca, 'XLim', [0 14]);
        set(gca, 'XTick', [1 2 3 4 5 6 7 8 9 10 11 12 13]);        
        xlabel('Index of datacenter (1 to 13)', 'FontSize', 20);
        ylabel('Number of opened servers', 'FontSize', 20);
        title(sprintf('(L_G = %d, L_R = %d); \t (|C| = %d); \t (k = 8) \t LAC', L_G(i)*2, L_R(i)*2, size(j)));
        grid on;
        savefig(strcat(file_name, '_LAC'));
        print(strcat(file_name, '_LAC'), '-dmeta');
    end
end