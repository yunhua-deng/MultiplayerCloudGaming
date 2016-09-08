%% cost
L_G = [75 150];
L_R = [50 100];
size = [10 50];

%y_max = [4.5 2]; % for basic problem
y_max = [4 2]; % for general problem

for j = 1:2
   for i = 1:2              
       file_name = sprintf('%d_%d_%d_2_costTotalMean', L_G(i), L_R(i), size(j));
       data_mean = importdata(file_name);       
       data_mean = [
           data_mean(1, 2:(end - 1));
           data_mean(2, 2:(end - 1));
           data_mean(3, 2:(end - 1));
           data_mean(4, 2:(end - 1))];
       
       file_name_std = sprintf('%d_%d_%d_2_costTotalStd', L_G(i), L_R(i), size(j));
       data_std = importdata(file_name_std);
       data_std = [
           data_std(1, 2:(end - 1));
           data_std(2, 2:(end - 1));
           data_std(3, 2:(end - 1));
           data_std(4, 2:(end - 1))
           ]; % excluding the last column
       
       figure;
       
       %bh = bar(data_mean);
       bh = barwitherr(data_std, data_mean); % with standard deviation bars
       
       color_1 = 'FloralWhite';
       color_2 = 'LightBlue';
       color_3 = 'LightGreen';
       color_4 = 'Khaki';
       color_5 = 'LightPink';
       color_6 = 'SandyBrown';
       
       bh(1).FaceColor = rgb(color_1);
       bh(2).FaceColor = rgb(color_2);
       bh(3).FaceColor = rgb(color_3);
       bh(4).FaceColor = rgb(color_4);
       bh(5).FaceColor = rgb(color_5);
       bh(6).FaceColor = rgb(color_6);
      
       set(gca, 'XTick', [1 2 3 4]);
       set(gca, 'XTickLabel', [2 4 6 8]);
       set(gca, 'YLim', [1 y_max(j)]);

       set(gca, 'fontsize', 16);
       xlabel('Server capacity: k', 'FontSize', 20);
       ylabel('Normalized cost', 'FontSize', 20);
       lh = legend('Random', 'Nearest', 'LSP', 'LBP', 'LCP', 'LCW', 'Orientation', 'vertical', 'Location', 'northwest');
       set(lh, 'FontSize', 14);       
       title(sprintf('(L_G = %d, L_R = %d); \t (|C| = %d)', L_G(i)*2, L_R(i)*2, size(j)));
       grid on;       
       pbaspect([2.5 1 1]);
       %pbaspect([2 1 1]);
       
       savefig(file_name);
       print(file_name, '-dmeta');
    end 
end

%% wastage
L_G = [75 150];
L_R = [50 100];
size = [10 50];

%y_max = [5 1]; % for basic problem
y_max = [4 1]; % for general problem

for j = 1:2
   for i = 1:2              
       file_name = sprintf('%d_%d_%d_2_capacityWastageMean', L_G(i), L_R(i), size(j));
       data_mean = importdata(file_name);       
       data_mean = [
           data_mean(1, 2:(end - 1));
           data_mean(2, 2:(end - 1));
           data_mean(3, 2:(end - 1));
           data_mean(4, 2:(end - 1))];
       
       file_name_std = sprintf('%d_%d_%d_2_capacityWastageStd', L_G(i), L_R(i), size(j));
       data_std = importdata(file_name_std);
      data_std = [
           data_std(1, 2:(end - 1));
           data_std(2, 2:(end - 1));
           data_std(3, 2:(end - 1));
           data_std(4, 2:(end - 1))
           ]; % excluding the last column
 
       figure;
       
       %bh = bar(data_mean);
       bh = barwitherr(data_std, data_mean);
       
       color_1 = 'FloralWhite';
       color_2 = 'LightBlue';
       color_3 = 'LightGreen';
       color_4 = 'Khaki';
       color_5 = 'LightPink';
       color_6 = 'SandyBrown';
       
       bh(1).FaceColor = rgb(color_1);
       bh(2).FaceColor = rgb(color_2);
       bh(3).FaceColor = rgb(color_3);
       bh(4).FaceColor = rgb(color_4);
       bh(5).FaceColor = rgb(color_5);
       bh(6).FaceColor = rgb(color_6);
      
       set(gca, 'XTick', [1 2 3 4]);
       set(gca, 'XTickLabel', [2 4 6 8]);
       set(gca, 'YLim', [0 y_max(j)]);

       set(gca, 'fontsize', 16);
       xlabel('Server capacity: k', 'FontSize', 20);
       ylabel('Capacity wastage ratio', 'FontSize', 20); 
       lh = legend('Random', 'Nearest', 'LSP', 'LBP', 'LCP', 'LCW', 'Orientation', 'vertical', 'Location', 'northwest');
       set(lh, 'FontSize', 14);       
       title(sprintf('(L_G = %d, L_R = %d); \t (|C| = %d)', L_G(i)*2, L_R(i)*2, size(j)));
       grid on;       
       pbaspect([2.5 1 1]);
       %pbaspect([2 1 1]);      
       
       savefig(file_name);
       print(file_name, '-dmeta');
    end 
end