% Poniższy program to próba stworzenia zwykłego kodera huffmana w celu
% lepszego zrozumienia algorytmu. Poprawność kodu niepewna - rozmiar
% obrazu pozostaje ten sam, a nie próbowałam robić dekodera. Po otwarciu
% pliku skompresowanego w gimpie niewiele widać i są tam tylko pojedyncze
% czarne piksele na białym tle, co jest trochę dziwne, ale z drugiej strony
% macierz wypluta przez program wygląda zgodnie z oczekiwaniami. Możliwe że
% powinnam wymusić inny typ danych wyjściowych. Bo liczba pikseli pozostaje
% w końcu taka sama...


% function non_adaptive_huffman(read_file_handle, write_file_handle)
% powyższą linijkę odkomentować jeśli chce się korzystać z programu jako
% funkcji, wówczas wywołuje się go z terminala np:
% non_adaptive_huffman ./obrazy_testowe/barbara.pgm ./output/barbara_compressed.pgm
% i należy wtedy zakomentować linijki 21 i 22 i odkomentować ostatnią linię
% programu (% end). Zrobiłam go domyślnie w obecnej formie, a nie jako
% funkcję, bo tak łatwiej debuggować.

% wynikowa tabela to result_data

% import danych i ustalenie nazwy pliku wyjściowego
read_file_handle = "./obrazy_testowe/lena.pgm";
write_file_handle = "./output/lena_compressed.pgm"; % uwaga! folder output trzeba utworzyć wcześniej
data = importdata(read_file_handle);
% image(data); % nie ogrania że to odcienie szarości i wyświetla kolorowy obraz w stylu Warhola

% zebranie statystyki
row_data = reshape(data, [], 1);
[weights, words] = groupcounts(row_data);
[weights_sorted, idx] = sort(weights);
words_sorted = words(idx);
weights = flipud(weights_sorted); % wagi węzłów
words = flipud(words_sorted); % wartości pikseli dla węzłów startowych, dla innych - 0

% algorytm huffmana

% inicjalizacja drzewa (drzewo w moim rozwiazaniu to tabela węzłów)
tree_size = size(words, 1);
node_values = zeros(tree_size, 1); % 0 lub 1. Domyślnie wszędzie 0. 0 także dla korzenia
parent_index = zeros(tree_size, 1); % indeks rodzica węzła. 0 jeśli brak
children_indexes = zeros(tree_size, 2); % indeksy dzieci węzła. 0 0 jeśli brak
level_numbers = ones(tree_size, 1); % poziom w drzewie - pole stworzone z myślą o huffmanie adaptacyjnym
indexes = (1:tree_size)'; % numer identyfikacyjny węzła
nodes = table(indexes, words, weights, node_values, level_numbers, parent_index, children_indexes);
initial_nodes_size = tree_size;
free_nodes = nodes;
num_of_free_nodes = tree_size;

all_coded = false;
i = 0;

% tworzenie wlasciwego drzewa
while ~all_coded

    % znajdz dwa najmniejsze wolne wezly
    [min_nodes_weigths, min_fake_idx] = mink(free_nodes.weights, 2);
    min_nodes = free_nodes(min_fake_idx, :);
    min_idx = min_nodes.indexes;

    % utworz nowy wezel rodzica
    index = tree_size + 1;
    word = 0;
    weight = min_nodes.weights(1) + min_nodes.weights(2);
    level = max(min_nodes.level_numbers) + 1;
    value = 0;
    parent = 0;
    children = [min_nodes.indexes(1), min_nodes.indexes(2)];
    new_node = {index, word, weight, value, level, parent, children};

    % aktualizuj drewo
    nodes = [nodes; new_node];
    tree_size = tree_size + 1;
    nodes.parent_index(min_idx(1)) = index;
    nodes.parent_index(min_idx(2)) = index;

    % aktualizuj informacje ktore wezly są wolne
    for j = 1 : 2
        toDelete = (free_nodes.indexes == min_idx(j)) ;
        free_nodes(toDelete,:) = [];
    end
    free_nodes = [free_nodes; new_node];
    num_of_free_nodes = num_of_free_nodes - 1;

    % przypisz 0 i 1 wlasciwym wezlom
    nodes.node_values(min_idx(2)) = 1;
    
    % sprawdzenie czy dotarlismy do korzenia
    if (num_of_free_nodes == 1)
        all_coded = true;
    end
    i = i + 1;
end

% sortowanie drzewa - do zrobienia przy adaptacyjnym

% zczytanie drzewa

code = zeros(initial_nodes_size, 2);
max_num_of_values = 0;

for i = 1:initial_nodes_size
    values = [];
    parent_not_zero = true;
    node = nodes(i, :);
    while (parent_not_zero)
        if (node.parent_index > 0)
            node = nodes(node.parent_index, :);
            if (i ~= 1)
                values = [values, node.node_values];
            end
        else
            parent_not_zero = false;
        end
        if (size(values, 2) > max_num_of_values)
            max_num_of_values = size(values, 2);
        end
    end
    values = flipud(values);

    % zamiana na liczbę binarną
    bin_str = num2str(values);
    bin_str = bin_str(~isspace(bin_str)); % usunięcie spacji
    % Zamiana ciągu binarnego na liczbę dziesiętną
    decimal_value = bin2dec(bin_str);
    number = decimal_value;
    row = [nodes(i, :).words, number];
    code(i, :) = row;
end

% kodowanie
result_data = zeros(size(data));
for i = 1:initial_nodes_size
    % adaptacja do dodania (możliwe że nie tu, w ogóle trzebaby chyba od nowa napisać ten adaptacyjny)
    result_data(data == code(i, 1)) = code(i, 2);
end


% image(result_data); % nie ogrania że to odcienie szarości i wyświetla kolorowy obraz w stylu Warhola

% zapis danych skompresowanych
imwrite(result_data, write_file_handle);

% end % patrz drugi komentarz na szczycie