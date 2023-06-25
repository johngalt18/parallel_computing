#include <iostream>
#include <fstream>
#include <vector>
#include <mpi.h>

const int DEAD = 0;
const int ALIVE = 1;

void loadConfiguration(const std::string& filename, std::vector<std::vector<int>>& grid, int& rows, int& cols) {
    std::ifstream inputFile(filename);
    if (!inputFile) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    inputFile >> rows >> cols;
    grid.resize(rows, std::vector<int>(cols, DEAD));

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            inputFile >> grid[i][j];
        }
    }

    inputFile.close();
}

void saveConfiguration(const std::string& filename, const std::vector<std::vector<int>>& grid, int rows, int cols) {
    std::ofstream outputFile(filename);
    if (!outputFile) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    outputFile << rows << " " << cols << std::endl;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            outputFile << grid[i][j] << " ";
        }
        outputFile << std::endl;
    }

    outputFile.close();
}

int countNeighbours(const std::vector<std::vector<int>>& grid, int row, int col) {
    int rows = grid.size();
    int cols = grid[0].size();
    int count = 0;

    for (int i = row - 1; i <= row + 1; i++) {
        for (int j = col - 1; j <= col + 1; j++) {
            if (i >= 0 && i < rows && j >= 0 && j < cols && !(i == row && j == col)) {
                count += grid[i][j];
            }
        }
    }

    return count;
}

void evolve(std::vector<std::vector<int>>& grid, int rank, int numProcs) {
    int rows = grid.size();
    int cols = grid[0].size();
    std::vector<std::vector<int>> newGrid(rows, std::vector<int>(cols));

    for (int i = rank; i < rows; i += numProcs) {
        for (int j = 0; j < cols; j++) {
            int neighbours = countNeighbours(grid, i, j);
            if (grid[i][j] == ALIVE) {
                if (neighbours < 2 || neighbours > 3) {
                    newGrid[i][j] = DEAD;
                } else {
                    newGrid[i][j] = ALIVE;
                }
            } else {
                if (neighbours == 3) {
                    newGrid[i][j] = ALIVE;
                } else {
                    newGrid[i][j] = DEAD;
                }
            }
        }
    }

    grid = newGrid;
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, numProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    std::string inputFilename = "input.txt";
    std::string outputFilename = "output.txt";
    int numSteps = 10;

    if (argc >= 2) {
        inputFilename = argv[1];
    }
    if (argc >= 3) {
        numSteps = std::stoi(argv[2]);
    }
    if (argc >= 4) {
        outputFilename = argv[3];
    }

    std::vector<std::vector<int>> grid;
    int rows, cols;

    if (rank == 0) {
        loadConfiguration(inputFilename, grid, rows, cols);
    }

    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

    grid.resize(rows, std::vector<int>(cols));

    MPI_Bcast(&grid[0][0], rows * cols, MPI_INT, 0, MPI_COMM_WORLD);

    for (int step = 0; step < numSteps; step++) {
        evolve(grid, rank, numProcs);
        MPI_Barrier(MPI_COMM_WORLD);
    }

    if (rank == 0) {
        saveConfiguration(outputFilename, grid, rows, cols);
    }

    MPI_Finalize();
    return 0;
}