#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Pizza bill and tip splitter with service rating
// Created 25/01/2026 By Alyx Deland
// Revision 2, Added data logging feature

// Function for tip and tax calculation
float math(float a, float b) {
    return (a * b) + a;
}

// Function to Validate ints
int getInt(const char *input) {
    int value;
    while (1) {
        printf("%s", input);
        // If user inputs an integer, return it
        if (scanf("%d", &value) == 1) {
            // Clear the input buffer
            while (getchar() != '\n');
            return value;
        }
        // If input is not an integer, print an error message and clear the input buffer
        printf("Invalid input, please try again.\n");
        while (getchar() != '\n');
    }
}

// Function to validate floats, identical to getInt aside from float data type
float getFloat(const char *input) {
    float value;
    while (1) {
        printf("%s", input);
        if (scanf("%f", &value) == 1) {
            while (getchar() != '\n');
            return value;
        }
        printf("Invalid input, please try again.\n");
        while (getchar() != '\n');
    }
}

// Function to get a 'yes' or 'no' answer using regex
int getConfirmation(const char *input) {
    regex_t regex;
    char buffer[100];

    // Note: <regex.h> is a POSIX standard and may not be available on all compilers,
    // especially the default MSVC compiler on Windows.
    // Compile regex to match 'y' or 'n' case-insensitively, anchored to the start and end of the string.
    if (regcomp(&regex, "^[yn]$", REG_EXTENDED | REG_ICASE)) {
        fprintf(stderr, "Could not compile regex\n");
        exit(1);
    }

    while (1) {
        printf("%s", input);
        if (fgets(buffer, sizeof(buffer), stdin)) {
            // Remove trailing newline character
            buffer[strcspn(buffer, "\n")] = 0;

            // Check if the input string matches the regular expression
            if (regexec(&regex, buffer, 0, NULL, 0) == 0) {
                char response = tolower(buffer[0]);
                regfree(&regex); // Free the compiled regex
                return (response == 'y'); // Return true (1) if 'y' or 'Y', false (0) otherwise.
            }
        }
        printf("Invalid input, please enter 'y' or 'n'.\n");
    }
}

// Function to open log file and store store data to it
void logSplit(FILE **fp, int tables_served, float table_total, int num_customers, float all_tips, float base_split) {
    // Open the file once and reuse the handle for all susequent logs
    if (*fp == NULL) {
        *fp = fopen("pizza_log.txt", "a");
        if (*fp == NULL) {
            perror("Error opening log file");
            return;
        }
    }
    // Log data to the file
    fprintf(*fp, "Table: %d, Table Total: %.2f, Customers: %d, Tip: %.2f, Per person: %.2f\n", tables_served, table_total, num_customers, all_tips, base_split);
}

// Main function
int main() {
    // Constant values
    const float tax = 0.13;
    const float shift_bonus = 0.12;

    // Non-loop variables
    float shift_total = 0;
    float total_rating = 0;
    int tables_served = 0;
    int next_table = 1;
    
    // Function pointers
    float (*m_ptr)(float, float) = math;
    FILE *log_file = NULL;
    void (*log_ptr)(FILE **, int, float, int, float, float) = logSplit;

    // Core loop
    while (next_table) {
        printf("\n--- New Table (#%d) ---\n", tables_served + 1);
        
        // Input loop for number of customers
        int num_customers;
        while (1) {
            num_customers = getInt("Number of customers at the table: ");
            if (num_customers < 3) {
                printf("There is a minimum of 3 customers required per table.\n");
            } else if (num_customers > 8) {
                printf("There is a maximum of 8 customers allowed per table.\n");
            } else {
                break;
            }            
        }

        // Get the bill amount for the table before tax and tip
        float original_bill = getFloat("Pre-tax bill: $");
        
        // Calculate base split and prepare for customer totals
        // Allocate memory for customer totals to avoid stack overflow
        float base_split = original_bill / num_customers;
        float *customer_totals = calloc(num_customers, sizeof(float));
        if (customer_totals == NULL) {
            printf("Error: Memory allocation failed. Exiting.\n");
            if(log_file != NULL) {
                fclose(log_file);
            }
            return 1;
        }
        float table_total = 0;
        float all_tips = 0;
        
        // Tip calculation loop
        for (int i = 0; i < num_customers; i++) {
            printf("\n--- Customer %d ---\n", i + 1);
            printf("Food share: $%.2f\n", base_split);
            int choice = getInt("Tip style:\n1: Dollar Amount\n2: Percentage\n3: No tip.\nSelect your tip style: ");
            
            float tip_amount = 0;

            // Get tip based on choice
            if (choice == 1) {
                tip_amount = getFloat("Tip amount: $");
            } else if (choice == 2) {
                float tip_percent = getFloat("Tip percent: ");
                tip_amount = base_split * (tip_percent / 100);
            } else if (choice == 3) {
                printf("No tip.\n");
            } else {
                printf("Invalid choice, no tip assumed.\n");
            }
            // Store customer totals, after tax and tip are added, as individual amounts in an array
            customer_totals[i] = m_ptr(base_split, tax) + tip_amount;
            // Combine the array for the table's total after tax and tip
            table_total += customer_totals[i];
            // Combine the total tips from all customers at the table
            all_tips += tip_amount;
        }

        // Service rating
        int rating = getInt("How would you rate the service today? (1-5 stars): ");

        // Trunk service rating if outside the traditional 1-5 stars
        if (rating < 1) rating = 1;
        if (rating > 5) rating = 5;

        // Calculate the total number of "stars" for this shift
        total_rating += rating;
        // Add this table's total to this shift's total
        shift_total += table_total;
        // Increment the number of tables served
        tables_served++;

        // Table's bill summary
        printf("\nTable Total: $%.2f\n", table_total);
        printf("Service Rating: %d\n", rating);
        printf("------------------------------\n");
        printf("Customer Totals:\n");
        // Print each customer's total
        for (int i = 0; i < num_customers; i++) {
            printf("Customer %d Total: $%.2f\n", i + 1, customer_totals[i]);
        }
        printf("------------------------------\n");

        // Log data to file
        log_ptr(&log_file, tables_served, table_total, num_customers, all_tips, base_split);

        // Free memory
        free(customer_totals);

        // Next table prompt
        next_table = getConfirmation("Is there another table to process? (y/n): ");
    }
    
    // Close the log file
    if(log_file != NULL) {
        fclose(log_file);
    }
    
    // Only calculate and display the summary if at least one table was served.
    if (tables_served > 0) {
        float extra = 0;
        float average_rating = total_rating / tables_served;
        // Calculate bonus if the average rating is high enough
        if (average_rating >= 4) {
            extra = shift_total * shift_bonus;
        }
        float take_home = shift_total + extra;
        // End of shift display
        printf("\n\n--- SHIFT SUMMARY ---");
        printf("\nTables Served: %d", tables_served);
        printf("\nBase Revenue: $%.2f", shift_total);
        printf("\nTotal Bonus: $%.2f", extra);
        printf("\nTotal Income: $%.2f", take_home);
        printf("\nAverage Rating: %.1f / 5 stars\n", average_rating);
    } else {
        printf("\nNo tables were served.\n");
    }

    return 0;
}