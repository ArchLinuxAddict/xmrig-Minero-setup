#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// Function to run a system command and check for success
int run_command(const char *command)
{
    int result = system(command);
    if (result != 0)
    {
        printf("Command '%s' failed with error code %d.\n", command, result);
        return result;
    }
    return 0;
}

// Function to replace a substring in a source string and store the result in the destination buffer
void replace_substring(const char *source, const char *old_substring, const char *new_substring, char *destination)
{
    const char *pos = source;
    char *dest = destination;
    size_t old_len = strlen(old_substring);
    size_t new_len = strlen(new_substring);

    while ((pos = strstr(source, old_substring)) != NULL)
    {
        size_t len = pos - source;
        strncpy(dest, source, len);
        dest += len;
        source = pos + old_len;
        strcpy(dest, new_substring);
        dest += new_len;
    }
    strcpy(dest, source);
}

void clear() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void get_distribution(char *distro, size_t size) {
    FILE *file = fopen("/etc/os-release", "r");
    if (!file) {
        perror("Failed to open /etc/os-release");
        strncpy(distro, "unknown", size);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "ID=", 3) == 0) {
            strncpy(distro, line + 3, size);
            // Remove trailing newline character
            distro[strcspn(distro, "\n")] = 0;
            break;
        }
    }

    fclose(file);
}

// Function to get the Linux distribution ID_LIKE value
void get_distribution_id_like(char *id_like, size_t size) {
    FILE *file = fopen("/etc/os-release", "r");
    if (!file) {
        perror("Failed to open /etc/os-release");
        strncpy(id_like, "unknown", size);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "ID_LIKE=", 8) == 0) {
            strncpy(id_like, line + 8, size);
            // Remove trailing newline character
            id_like[strcspn(id_like, "\n")] = 0;
            break;
        }
    }

    fclose(file);
}

// Function to install packages based on the Linux distribution
void install_packages(const char *distro, const char *id_like) {
    if (strcmp(distro, "ubuntu") == 0 || strcmp(distro, "debian") == 0) {
        printf("Detected Ubuntu/Debian. Installing packages...\n");
        run_command("sudo apt update -y && sudo apt-get install -y build-essential cmake libuv1-dev libssl-dev libhwloc-dev");
    } else if (strcmp(distro, "fedora") == 0 || strcmp(distro, "centos") == 0) {
        printf("Detected Fedora/CentOS. Installing packages...\n");
        run_command("sudo dnf install -y make automake gcc gcc-c++ kernel-devel cmake libuv-devel openssl-devel hwloc-devel");
    } else if (strcmp(distro, "arch") == 0 || strcmp(distro, "manjaro") == 0 || strcmp(id_like, "arch") == 0) {
        printf("Detected Arch/Manjaro or derivative. Installing packages...\n");
        run_command("sudo pacman -Syu --needed base-devel cmake libuv openssl hwloc");
    } else {
        printf("Unknown or unsupported distribution: %s\n", distro);
    }
}

void build_xmrig(const char *buildType) {
    if (strcmp(buildType, "basic") == 0) {
        printf("Building XMRig with Basic configuration...\n");
        run_command("mkdir -p xmrig/build && cd xmrig/build && cmake .. && make -j$(nproc)");
    } else if (strcmp(buildType, "advanced") == 0) {
        printf("Building XMRig with Advanced configuration...\n");
        run_command("mkdir -p xmrig/build && cd xmrig/scripts && ./build_deps.sh && cd ../build && cmake .. -DXMRIG_DEPS=scripts/deps && make -j$(nproc)");
    } else if (strcmp(buildType, "cuda") == 0) {
        printf("Building XMRig with CUDA configuration...\n");
        run_command("git clone https://github.com/xmrig/xmrig-cuda.git && mkdir -p xmrig-cuda/build && cd xmrig-cuda/build && cmake .. -DCUDA_LIB=/usr/local/cuda/lib64/stubs/libcuda.so -DCUDA_TOOLKIT_ROOT_DIR=/usr/local/cuda && make -j$(nproc)");
    } else {
        printf("Invalid build type.\n");
    }
}

void run()
{
    char donation[4];

    clear();
    printf("Do you want to set donations to zero? (yes/no): ");
    scanf("%3s", donation);

    // Convert donation input to lowercase
    for (int i = 0; donation[i]; i++)
    {
        donation[i] = tolower(donation[i]);
    }

    if (strcmp(donation, "yes") == 0)
    {
        printf("Setting donation to 0...\n");

        const char *filename = "xmrig/src/donate.h";
        FILE *file = fopen(filename, "r");
        if (!file)
        {
            perror("Failed to open file");
            return;
        }

        // Read the entire file into a buffer
        char buffer[1024 * 16]; // Assuming the file size is less than 16KB
        size_t length = fread(buffer, 1, sizeof(buffer) - 1, file);
        buffer[length] = '\0';
        fclose(file);

        // Buffers for modified content
        char modified_buffer1[1024 * 16];
        char modified_buffer2[1024 * 16];

        // Replace the required substrings
        replace_substring(buffer, "constexpr const int kDefaultDonateLevel = 1;", "constexpr const int kDefaultDonateLevel = 0;", modified_buffer1);
        replace_substring(modified_buffer1, "constexpr const int kMinimumDonateLevel = 1;", "constexpr const int kMinimumDonateLevel = 0;", modified_buffer2);

        // Write the modified buffer back to the file
        file = fopen(filename, "w");
        if (!file)
        {
            perror("Failed to open file for writing");
            return;
        }
        fwrite(modified_buffer2, 1, strlen(modified_buffer2), file);
        fclose(file);

        printf("Donation levels have been set to 0 successfully.\n");
    }
}

int main()
{
    char distro[256];
    char id_like[256];
    char response[4];
    char build[4];
    int choice;
    
    clear();
    get_distribution(distro, sizeof(distro));
    get_distribution_id_like(id_like, sizeof(id_like));
    printf("Detected distribution: %s\n", distro);
    printf("ID_LIKE: %s\n", id_like);

    install_packages(distro, id_like);
    clear();
    printf("Do you want to set up xmrig? (yes/no): ");
    scanf("%3s", response);

    // Convert response to lowercase
    for (int i = 0; response[i]; i++)
    {
        response[i] = tolower(response[i]);
    }

    if (strcmp(response, "yes") == 0)
    {
        printf("Continuing with xmrig setup...\n");
        if (run_command("git clone https://github.com/xmrig/xmrig.git") != 0)
        {
            printf("Failed to clone xmrig repository.\n");
            return 1;
        }
        printf("xmrig downloaded successfully.\n");
        run();
        clear();
        printf("Do you want to compile xmrig? (yes/no): ");
        scanf("%3s", build);

        for (int i = 0; build[i]; i++)
        {
            build[i] = tolower(build[i]);
        }
        if (strcmp(build, "yes") == 0)
        {
            clear();
            printf("How would you like to build XMRig?\n");
            printf("1. Basic: Standard build without any special optimizations.\n");
            printf("2. Advanced: Includes additional optimizations and features.\n");
            printf("3. CUDA: Optimized for NVIDIA GPUs using CUDA.\n");
            printf("Please enter the number corresponding to your choice (1, 2, or 3): ");

            if (scanf("%d", &choice) != 1) {
                printf("Invalid input. Please enter a number.\n");
                return 1;
            }

            switch (choice) {
                case 1:
                    printf("You have chosen Basic build.\n");
                    build_xmrig("basic");
                    break;
                case 2:
                    printf("You have chosen Advanced build.\n");
                    build_xmrig("advanced");
                    break;
                case 3:
                    printf("You have chosen CUDA build.\n");
                    build_xmrig("cuda");
                    break;
                default:
                    printf("Invalid choice. Please enter 1, 2, or 3.\n");
                    return 1;
            }
            clear();
            printf("Done setup and build thanks for using the software\n");
        }
    }
    else
    {
        printf("Exiting the setup.\n");
    }

    return 0;
}