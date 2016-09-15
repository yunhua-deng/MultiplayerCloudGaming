%% ratio over total
set(gcf, 'Position', get(0, 'Screensize'));
L_G = [25 50 100];
for i = 1:3
    subplot(1, 3, i);
    simple = importdata(sprintf('simple_%d.csv', L_G(i)));
    hold on;
    plot(simple(:, 1), simple(:, 3), '-^', 'LineWidth', 1);   
    nearest = importdata(sprintf('nearest_%d.csv', L_G(i)));    
    plot(nearest(:, 1), nearest(:, 3), '-o', 'LineWidth', 1);
    random = importdata(sprintf('random_%d.csv', L_G(i)));
    plot(random(:, 1), random(:, 3), '-x', 'LineWidth', 1);
    plot(random(:, 1), random(:, 2), '-*', 'LineWidth', 1); % plot the eligible once
    
    lg = legend('simple', 'nearest', 'random', 'eligible', 'Location', 'southeast');
    set(lg, 'FontSize', 12);    
    set(gca, 'YLim', [0 1]);
    set(gca, 'fontsize', 12);
    xlabel('total number of clients', 'fontsize', 14);
    ylabel('ratio over total', 'fontsize', 14);
    title(sprintf('one-way delay threshold: %d msec', L_G(i)));
    pbaspect([2 1 1]);
    box on;
    grid on;
end

%% ratio over eligible
set(gcf, 'Position', get(0, 'Screensize'));
L_G = [25 50 100];
 set(gcf, 'Position', get(0, 'Screensize'));
for i = 1:3
    subplot(1, 3, i);
    simple = importdata(sprintf('simple_%d.csv', L_G(i)));    
    plot(simple(:, 1), simple(:, 3)./simple(:, 2), '-^', 'LineWidth', 1);
    hold on;
    nearest = importdata(sprintf('nearest_%d.csv', L_G(i)));    
    plot(nearest(:, 1), nearest(:, 3)./nearest(:, 2), '-o', 'LineWidth', 1);
    random = importdata(sprintf('random_%d.csv', L_G(i)));
    plot(random(:, 1), random(:, 3)./random(:, 2), '-x', 'LineWidth', 1);
    
    lg = legend('simple', 'nearest', 'random', 'Location', 'southeast');
    set(lg, 'FontSize', 12);    
    set(gca, 'YLim', [0 1]);
    set(gca, 'fontsize', 12);
    xlabel('total number of clients', 'fontsize', 14);
    ylabel('ratio over eligible', 'fontsize', 14);
    title(sprintf('one-way delay threshold: %d msec', L_G(i)));
    pbaspect([2 1 1]);
    grid on;
    box on;
end