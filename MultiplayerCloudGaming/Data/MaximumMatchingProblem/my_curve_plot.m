L_G = [25 50 100];
for i = 1:3
    subplot(3, 1, i);
    nearest = importdata(sprintf('nearest_%d.csv', L_G(i)));
    plot(nearest(:, 1), nearest(:, 2), '-', 'LineWidth', 1);
    hold on;
    plot(nearest(:, 1), nearest(:, 3), '-o', 'LineWidth', 1);
    random = importdata(sprintf('random_%d.csv', L_G(i)));
    plot(random(:, 1), random(:, 3), '-x', 'LineWidth', 1);
    lg = legend('Eligible', 'Nearest', 'Random', 'Location', 'southeast');
    set(lg, 'FontSize', 14);
    grid on;
    set(gca, 'YLim', [0 1]);
    set(gca, 'fontsize', 12);
    xlabel('total number of clients for grouping (X)', 'fontsize', 14);
    ylabel('ratio over X', 'fontsize', 14);
    title(sprintf('one-way delay threshold: %d msec', L_G(i)));
    pbaspect([2 1 1]);
end
savfig('nearest_vs_random');
