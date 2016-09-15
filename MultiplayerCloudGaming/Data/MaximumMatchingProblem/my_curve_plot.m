set(gcf, 'Position', get(0, 'Screensize'));
L_T = [25 50 100];
S_S = [10 20 40];
for i = 1:3
    %eligible
    subplot(3, 4, (i - 1) * 4 + 1);
    simple = importdata(sprintf('simple_%d_%d.csv', L_T(i), S_S(1)));
    plot(simple(:, 1), simple(:, 2), '-xk', 'LineWidth', 2);
    set(gca, 'YLim', [0 1], 'fontsize', 10);
    xlabel('total number of clients', 'fontsize', 14);
    ylabel('ratio of eligible over total', 'fontsize', 14);    
    title(sprintf('latencyThreshold = %d', L_T(i)));    
    grid on;
    box on;
    pbaspect([1.5 1 1]);
    
    %algorithm
    for j = 1:3
        subplot(3, 4, (i - 1) * 4 + 1 + j);
        
        simple = importdata(sprintf('simple_%d_%d.csv', L_T(i), S_S(j)));
        plot(simple(:, 1), simple(:, 3)./simple(:, 2), '-^', 'LineWidth', 0.5);
        
        hold on;
        
        simple = importdata(sprintf('simple-sort-fewer_%d_%d.csv', L_T(i), S_S(j)));
        plot(simple(:, 1), simple(:, 3)./simple(:, 2), '-s', 'LineWidth', 0.5);
        
        simple = importdata(sprintf('simple-sort-more_%d_%d.csv', L_T(i), S_S(j)));
        plot(simple(:, 1), simple(:, 3)./simple(:, 2), '-o', 'LineWidth', 0.5);
        
        nearest = importdata(sprintf('nearest_%d_%d.csv', L_T(i), S_S(j)));    
        plot(nearest(:, 1), nearest(:, 3)./nearest(:, 2), '-h', 'LineWidth', 0.5);
        
        random = importdata(sprintf('random_%d_%d.csv', L_T(i), S_S(j)));
        plot(random(:, 1), random(:, 3)./random(:, 2), '-*', 'LineWidth', 0.5);
        
        if i == 1 && j == 1
            lg = legend('simple', 'simple-sort-fewer', 'simple-sort-more', 'nearest', 'random', 'Location', 'southeast');
            set(lg, 'FontSize', 10);
        end
        
        set(gca, 'YLim', [0 1], 'fontsize', 10);
        xlabel('total number of clients', 'fontsize', 14);
        ylabel('ratio of grouped over eligible', 'fontsize', 14);
        title(sprintf('latencyThreshold = %d | sessionSize = %d', L_T(i), S_S(j)));        
        grid on;
        box on;        
        pbaspect([1.5 1 1]);
    end
end