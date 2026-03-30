#include <stdio.h>
#include <stdlib.h>
// Pizza bill and tip splitter with service rating
// Created by Alyx Deland 20/01/2026
// Rev Hist: 
// A01: Inital program
// A02: Added data logging and memory allocation
// A03: Better input validation, more pointers

// Function for tip and tax calculation
float math(float a, float b) {
    return (a * b) + a;
}

// Functions to Validate ints and floats
int getInt(const char *input, int max) {
    int value;
    while (1) {
        printf("%s", input);
        // If user inputs an integer, return it
        if (scanf("%d", &value) == 1) {
            // Clear the input buffer
            while (getchar() != '\n');
            // Check if the input integer is within the desired range (0 to max)
            if (value > 0 && value <= max) {
                return value; // Input is valid and within the specified range
            } else {
                // Input is an integer but outside the desired range
                if (value <= 0) {
                    printf("Number too low, please try again.\n");
                } else { // value > max
                    printf("Number too high (must be %d or less), please try again.\n", max);
                }
            }
        } else {
            // If input is not an integer, print an error message and clear the input buffer
            printf("Invalid input, please try again.\n");
            while (getchar() != '\n');
        }
    }
}

// Function to validate floats, identical to getInt save for float data type
// Float data type used because double precission is not necessary for this program
float getFloat(const char *input, int max) {
    float value;
    while (1) {
        printf("%s", input);
        if (scanf("%f", &value) == 1) {
            while (getchar() != '\n');
            if (value > 0 && value <= max) {
                return value;
            } else {
                if (value <= 0) {
                    printf("Number too low, please try again.\n");
                } else {
                    printf("Number too high (must be %.2f or less), please try again.\n", max);
                }
            }
        } else {
            printf("Invalid input, please try again.\n");
            while (getchar() != '\n');
        }
    }
}

// Function to open log file and store store data to it
void log_split(FILE **fp, int tables_served, float table_total, int num_customers, float all_tips, float base_split) {
    // Open the file once and reuse the handle for all susequent logs
    if (*fp == NULL) {
        *fp = fopen("pizza_log.txt", "a");
        if (*fp == NULL) {
            perror("Error opening log file");
            return;
        }
    }
    // Log data to the file
    fprintf(*fp, "Table: %d, Table Total: %.2f, Size: %d, Tip: %.2f, Per person: %.2f\n", tables_served, table_total, num_customers, all_tips, base_split);
    // Matches assignment 1 spec, names are altered to
    // better align with SET coding standards' naming conventions
    // and to work with my program's variables
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
    char next_table = 'y';

    // Function/ File pointers
    int (*i_ptr)(const char *, int) = getInt;
    float (*f_ptr)(const char *, int) = getFloat;
    float (*m_ptr)(float, float) = math;
    FILE *log_file = NULL;

    // Core loop
    while (next_table == 'y' || next_table == 'Y') {
        printf("\n--- New Table (#%d) ---\n", tables_served + 1);
        
        // Input loop for number of customers, break when 
        int num_customers;
        while (1) {
            // Maximum allowed number of customers at a table is 8, 
            // and minimum number of customers is 3, as per assignment 1 spec
            num_customers = i_ptr("Number of customers at the table (3-8): ", 8);
            if (num_customers < 3) {
                printf("There is a minimum of 3 customers required per table.\n");
            } else {
                break;
            }            
        }

        // Get the bill amount for the table before tax and tip
        float original_bill = f_ptr("Pre-tax bill: $", 10000); // Assuming a max bill of 10000
        
        // Calculate base split and prepare for customer totals
        // Allocate memory for customer totals to avoid stack overflow
        float base_split = original_bill / num_customers;
        float *customer_totals = calloc(num_customers, sizeof(float));
        if (customer_totals == NULL) {
            printf("Error: Memory allocation failed. Exiting.\n");
            if(log_file != NULL) {
                fclose(log_file); 
            } // Close the log file if memory allocation fails and the file is open, then end the program
            return 1;
        }
        float table_total = 0;
        float all_tips = 0;
        
        // Tip calculation loop
        for (int i = 0; i < num_customers; i++) {
            // Display options to each customer iteratively
            printf("\n--- Customer %d ---\n", i + 1);
            printf("Food share: $%.2f\n", base_split);
            int choice = i_ptr("Tip style:\n1: Dollar Amount\n2: Percentage\n3: No tip.\nSelect your tip style: ", 3);
            
            float tip_amount = 0;

            // Get tip based on choice
            if (choice == 1) { // Dollar amount
                tip_amount = f_ptr("Tip amount: $", 1000); // Assuming max tip of $1000
            } else if (choice == 2) {
                float tip_percent = i_ptr("Tip percent: ", 100); // Assuming max tip percentage of 100%
                tip_amount = base_split * (tip_percent / 100);
            } else if (choice == 3) {
                printf("No tip.\n");
            } else {
                printf("Invalid choice, no tip assumed.\n");
            }
            // Store customer totals, after tax and tip are added,
            // as individual amounts in an allocated array
            customer_totals[i] = m_ptr(base_split, tax) + tip_amount;
            // Combine values in the array for the table's total after tax and tip
            table_total += customer_totals[i];
            // Combine the total tips from all customers at the table
            all_tips += tip_amount;
        }

        // Service rating, currently set as 1-5, 1-3 can easily be implemented  
        // if necessary though 1-5 is the standard for the industry
        int rating = getInt("How would you rate the service today? (1-5 stars): ", 5);

        // Calculate the total number of "stars" for this shift (used for average rating)
        total_rating += rating;
        // Add this table's total to this shift's total
        shift_total += table_total;
        // Increment the number of tables served
        tables_served++;

        // Table's bill summary print statements
        printf("\nTable Total: $%.2f\n", table_total);
        printf("Service Rating: %d\n", rating);
        printf("------------------------------\n");
        printf("Customer Totals:\n");
        // Print each customer's total, iteratively
        for (int i = 0; i < num_customers; i++) {
            printf("Customer %d Total: $%.2f\n", i + 1, customer_totals[i]);
        }
        printf("------------------------------\n");

        // Log data to file
        log_split(&log_file, tables_served, table_total, num_customers, all_tips, base_split);

        // Free memory
        free(customer_totals);

        // Next table prompt
        while (1) {
            printf("Is there another table to process? (y/n): ");
            if (scanf(" %c", &next_table) == 1) {
                // Clear the rest of the input buffer.
                while (getchar() != '\n');
                if (next_table == 'y' || next_table == 'Y') {
                    break; // If y or Y are inputed, break this loop and restart the main loop
                } else if (next_table == 'n' || next_table == 'N') {
                    next_table = 'n';
                    break; // if n or N are inputed, set next_table as
                    // n to break the main loop, and break this loop
                } else {
                    printf("Invalid input, please try again.\n"); 
                }
            } else {
                printf("Invalid input, please try again.\n");
            } // If anything other than y/Y/n/N is inputted, throw an error and retry
        }
    }
    
    // Close the log file
    if(log_file != NULL) {
        fclose(log_file);
    }
    
    // Only calculate and display the summary if at least one table was served.
    if (tables_served > 0) {
        // Init extra, calculate the staffer's average rating across tables this shift
        // Deduct 30% from shift total so restaurant makes some money too
        float extra = 0;
        float average_rating = total_rating / tables_served;
        float staff_total = shift_total * 0.7;
        // Calculate bonus if the average rating is high enough, 0 if not high enough
        if (average_rating >= 4) {
            extra = staff_total * shift_bonus;
        } else {
            extra = 0;
        }
        // Calculate the staffer's take home amount
        float take_home = staff_total + extra;
        // End of shift display statements
        printf("\n\n--- SHIFT SUMMARY ---");
        printf("\nTables Served: %d", tables_served);
        if (average_rating >= 4) { 
            // If the average rating is above a given value,
            // add these two lines to the summary
            printf("\nBase Revenue: $%.2f", staff_total);
            printf("\nBonus: $%.2f", extra);
        } else { // If not, add this line to the summary
            printf("\nNo Bonus earned.");
        }
 