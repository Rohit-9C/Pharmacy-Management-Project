#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SUPPLIERS 10
#define REORDER_MIN 10
#define REORDER_MAX 100
#define EXPIRY_ALERT_DAYS 30

typedef struct Supplier {
    int supplierID;
    char supplierName[50];
    int quantitySupplied;
    char contactInfo[50];
    struct Supplier* next;
} Supplier;


typedef struct Medication {
    int medicationID;
    char medicationName[50];
    int batchNumber;
    int quantityInStock;
    float pricePerUnit;
    char expirationDate[11]; // DD-MM-YYYY format
    int reorderLevel;
    int totalSales;
    Supplier* supplierList; 
    struct Medication* next;
} Medication;

// Global linked list for medications
Medication* medicationList = NULL;

// Function prototypes
void loadDataFromFile();
void addMedication();
void printPharmacyData();
void saveDataToFile();
void updateMedication();
void deleteMedication();
void searchMedication();
void generateStockAlerts();
void checkExpirationDates();
void sortMedicationsByExpiration();
void salesTracking(); 
void supplierManagement(); 
void findAllRounderSuppliers();
void findLargestTurnoverSuppliers(); 


int main() {
    int choice;
    loadDataFromFile(); 

    while (1) {
        printf("\nPharmacy Inventory Management System\n");
        printf("1. Add New Medication\n");
        printf("2. Update Medication Details\n");
        printf("3. Print Pharmacy Data\n");
        printf("4. Delete Medication \n ");
        printf("5. search medication \n");
        printf("6. Stock Alert of Medication \n ");
        printf("7. Check Expiration Dates \n");
        printf("8. Sort Medication by Expiration Date: \n ");
        printf("9. Sales Tracking\n");
        printf("10. Supplier Management \n ");
        printf("11. find all-rounder suppliers \n");
        printf("12. find supplier with largest turn-over\n");
        printf("13. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: addMedication(); break;
            case 2: updateMedication(); break;
            case 3: printPharmacyData(); break;
            case 4: deleteMedication(); break;
            case 5: searchMedication(); break;
            case 6: generateStockAlerts(); break;
            case 7: checkExpirationDates(); break;
            case 8: sortMedicationsByExpiration(); break;
            case 9: salesTracking(); break;
            case 10: supplierManagement(); break;
            case 11: findAllRounderSuppliers(); break;
            case 12: findLargestTurnoverSuppliers(); break;
            case 13: return 0;

            default: printf("Invalid choice! Please try again.\n");
        }
    }
    return 0;
}



void loadDataFromFile() {
    FILE* file = fopen("pharmacy_data.txt", "r");
    if (file == NULL) {
        printf("No existing data file found. Starting with an empty inventory.\n");
        return;
    }

    Medication* current = NULL;
    while (!feof(file)) {
        Medication* newMedication = (Medication*)malloc(sizeof(Medication));
        if (fscanf(file, "%d %s %d %d %f %s %d %d",
                   &newMedication->medicationID, newMedication->medicationName,
                   &newMedication->batchNumber, &newMedication->quantityInStock,
                   &newMedication->pricePerUnit, newMedication->expirationDate,
                   &newMedication->reorderLevel, &newMedication->totalSales) != 8) {
            free(newMedication);
            break;
        }

        newMedication->supplierList = NULL;
        newMedication->next = NULL;

        if (medicationList == NULL) {
            medicationList = newMedication;
        } else {
            current->next = newMedication;
        }
        current = newMedication;

        while (1) {
            Supplier* newSupplier = (Supplier*)malloc(sizeof(Supplier));
            if (fscanf(file, "%d %s %d %s",
                       &newSupplier->supplierID, newSupplier->supplierName,
                       &newSupplier->quantitySupplied, newSupplier->contactInfo) != 4) {
                free(newSupplier);
                break;
            }

            newSupplier->next = newMedication->supplierList;
            newMedication->supplierList = newSupplier;

            char buffer[20];
            fscanf(file, "%s", buffer);
            if (strcmp(buffer, "END_SUPPLIERS") == 0) {
                break;
            }
        }
    }

    fclose(file);
    printf("Data loaded from file successfully!\n");
}


void addMedication() {
    Medication* newMedication = (Medication*)malloc(sizeof(Medication));
    printf("Enter Medication ID: ");
    scanf("%d", &newMedication->medicationID);
    printf("Enter Medication Name: ");
    scanf("%s", newMedication->medicationName);
    printf("Enter Batch Number: ");
    scanf("%d", &newMedication->batchNumber);
    printf("Enter Quantity in Stock: ");
    scanf("%d", &newMedication->quantityInStock);
    printf("Enter Price per Unit: ");
    scanf("%f", &newMedication->pricePerUnit);
    printf("Enter Expiration Date (DD-MM-YYYY): ");
    scanf("%s", newMedication->expirationDate);
    printf("Enter Reorder Level: ");
    scanf("%d", &newMedication->reorderLevel);
    
    // Adding supplier information
    Supplier* newSupplier = (Supplier*)malloc(sizeof(Supplier));
    printf("Enter Supplier ID: ");
    scanf("%d", &newSupplier->supplierID);
    printf("Enter Supplier Name: ");
    scanf("%s", newSupplier->supplierName);
    printf("Enter Quantity Supplied: ");
    scanf("%d", &newSupplier->quantitySupplied);
    printf("Enter Supplier Contact Info: ");
    scanf("%s", newSupplier->contactInfo);
    newSupplier->next = NULL;

    // Link supplier to medication
    newMedication->supplierList = newSupplier;
    newMedication->totalSales = 0;
    newMedication->next = medicationList;
    medicationList = newMedication;
    printf("Medication added successfully!\n");

    saveDataToFile();
}



void saveDataToFile() {
    FILE* file = fopen("pharmacy_data.txt", "w");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    Medication* current = medicationList;
    while (current != NULL) {
        // Write medication details to file
        fprintf(file, "%d %s %d %d %.2f %s %d %d\n",
                current->medicationID, current->medicationName, current->batchNumber,
                current->quantityInStock, current->pricePerUnit, current->expirationDate,
                current->reorderLevel, current->totalSales);

        // Write supplier details to file
        Supplier* supplier = current->supplierList;
        while (supplier != NULL) {
            fprintf(file, "%d %s %d %s\n",
                    supplier->supplierID, supplier->supplierName,
                    supplier->quantitySupplied, supplier->contactInfo);
            supplier = supplier->next;
        }
        fprintf(file, "END_SUPPLIERS\n");

        current = current->next;
    }

    fclose(file);
    printf("Data saved to file successfully!\n");
}


void updateMedication() {
    int medicationID, batchNumber;
    printf("Enter Medication ID to update: ");
    scanf("%d", &medicationID);
    printf("Enter Batch Number to update: ");
    scanf("%d", &batchNumber);

    Medication* current = medicationList;
    while (current != NULL) {
        if (current->medicationID == medicationID && current->batchNumber == batchNumber) {
            printf("Medication found! Enter new details:\n");

    
            printf("Enter new Medication Name: ");
            scanf("%s", current->medicationName);
            printf("Enter new Quantity in Stock: ");
            scanf("%d", &current->quantityInStock);
            printf("Enter new Price per Unit: ");
            scanf("%f", &current->pricePerUnit);
            printf("Enter new Expiration Date (DD-MM-YYYY): ");
            scanf("%s", current->expirationDate);
            printf("Enter new Reorder Level: ");
            scanf("%d", &current->reorderLevel);

            printf("Do you want to update supplier information? (1 for Yes, 0 for No): ");
            int updateSupplier;
            scanf("%d", &updateSupplier);

            if (updateSupplier == 1) {
                Supplier* supplier = current->supplierList;
                while (supplier != NULL) {
                    printf("Supplier ID: %d\n", supplier->supplierID);
                    printf("Enter new Supplier Name: ");
                    scanf("%s", supplier->supplierName);
                    printf("Enter new Quantity Supplied: ");
                    scanf("%d", &supplier->quantitySupplied);
                    printf("Enter new Contact Information: ");
                    scanf("%s", supplier->contactInfo);

                    supplier = supplier->next;
                }
            }

            printf("Medication details updated successfully!\n");

            saveDataToFile();
            return;
        }
        current = current->next;
    }

    printf("Medication with ID %d and Batch Number %d not found!\n", medicationID, batchNumber);
}



void deleteMedication() {
    int medicationID, batchNumber;
    printf("Enter Medication ID to delete: ");
    scanf("%d", &medicationID);
    printf("Enter Batch Number to delete: ");
    scanf("%d", &batchNumber);

    Medication* current = medicationList;
    Medication* previous = NULL;

    while (current != NULL) {
        if (current->medicationID == medicationID && current->batchNumber == batchNumber) {
            if (previous == NULL) {
                // If the medication to delete is the first node
                medicationList = current->next;
            } else {
                // If the medication to delete is in the middle or end
                previous->next = current->next;
            }

            // Free memory allocated for suppliers
            Supplier* supplier = current->supplierList;
            while (supplier != NULL) {
                Supplier* temp = supplier;
                supplier = supplier->next;
                free(temp);
            }

            free(current);
            printf("Medication deleted successfully!\n");

            saveDataToFile();
            return;
        }

        previous = current;
        current = current->next;
    }

    printf("Medication with ID %d and Batch Number %d not found!\n", medicationID, batchNumber);
}


void searchMedication() {
    int choice;
    printf("Search by:\n");
    printf("1. Medication ID\n");
    printf("2. Medication Name\n");
    printf("3. Supplier Name\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    if (choice == 1) {
        int medicationID;
        printf("Enter Medication ID: ");
        scanf("%d", &medicationID);

        Medication* current = medicationList;
        while (current != NULL) {
            if (current->medicationID == medicationID) {
                printf("Medication found!\n");
                printf("Medication ID: %d\n", current->medicationID);
                printf("Medication Name: %s\n", current->medicationName);
                printf("Batch Number: %d\n", current->batchNumber);
                printf("Quantity in Stock: %d\n", current->quantityInStock);
                printf("Price per Unit: %.2f\n", current->pricePerUnit);
                printf("Expiration Date: %s\n", current->expirationDate);
                printf("Reorder Level: %d\n", current->reorderLevel);
                printf("Total Sales: %d\n", current->totalSales);

                Supplier* supplier = current->supplierList;
                while (supplier != NULL) {
                    printf("Supplier ID: %d\n", supplier->supplierID);
                    printf("Supplier Name: %s\n", supplier->supplierName);
                    printf("Quantity Supplied: %d\n", supplier->quantitySupplied);
                    printf("Contact Info: %s\n", supplier->contactInfo);
                    supplier = supplier->next;
                }
                return;
            }
            current = current->next;
        }
        printf("Medication with ID %d not found!\n", medicationID);
    } else if (choice == 2) {
        char medicationName[50];
        printf("Enter Medication Name: ");
        scanf("%s", medicationName);

        Medication* current = medicationList;
        while (current != NULL) {
            if (strcmp(current->medicationName, medicationName) == 0) {
                printf("Medication found!\n");
                printf("Medication ID: %d\n", current->medicationID);
                printf("Medication Name: %s\n", current->medicationName);
                printf("Batch Number: %d\n", current->batchNumber);
                printf("Quantity in Stock: %d\n", current->quantityInStock);
                printf("Price per Unit: %.2f\n", current->pricePerUnit);
                printf("Expiration Date: %s\n", current->expirationDate);
                printf("Reorder Level: %d\n", current->reorderLevel);
                printf("Total Sales: %d\n", current->totalSales);

                Supplier* supplier = current->supplierList;
                while (supplier != NULL) {
                    printf("Supplier ID: %d\n", supplier->supplierID);
                    printf("Supplier Name: %s\n", supplier->supplierName);
                    printf("Quantity Supplied: %d\n", supplier->quantitySupplied);
                    printf("Contact Info: %s\n", supplier->contactInfo);
                    supplier = supplier->next;
                }
                return;
            }
            current = current->next;
        }
        printf("Medication with Name %s not found!\n", medicationName);
    } else if (choice == 3) {
        char supplierName[50];
        printf("Enter Supplier Name: ");
        scanf("%s", supplierName);

        Medication* current = medicationList;
        int found = 0;

        while (current != NULL) {
            Supplier* supplier = current->supplierList;
            while (supplier != NULL) {
                if (strcmp(supplier->supplierName, supplierName) == 0) {
                    printf("Medication found!\n");
                    printf("Medication ID: %d\n", current->medicationID);
                    printf("Medication Name: %s\n", current->medicationName);
                    printf("Batch Number: %d\n", current->batchNumber);
                    printf("Quantity in Stock: %d\n", current->quantityInStock);
                    printf("Price per Unit: %.2f\n", current->pricePerUnit);
                    printf("Expiration Date: %s\n", current->expirationDate);
                    printf("Reorder Level: %d\n", current->reorderLevel);
                    printf("Total Sales: %d\n", current->totalSales);

                    printf("Supplier ID: %d\n", supplier->supplierID);
                    printf("Supplier Name: %s\n", supplier->supplierName);
                    printf("Quantity Supplied: %d\n", supplier->quantitySupplied);
                    printf("Contact Info: %s\n", supplier->contactInfo);
                    found = 1;
                }
                supplier = supplier->next;
            }
            current = current->next;
        }

        if (!found) {
            printf("No medication found for supplier %s!\n", supplierName);
        }
    } else {
        printf("Invalid choice!\n");
    }
}


void generateStockAlerts() {
    Medication* current = medicationList;
    int alertCount = 0;

    printf("\nStock Alerts:\n");
    printf("------------------------------------------------------------\n");

    while (current != NULL) {
        if (current->quantityInStock < current->reorderLevel) {
            printf("Medication ID: %d\n", current->medicationID);
            printf("Medication Name: %s\n", current->medicationName);
            printf("Batch Number: %d\n", current->batchNumber);
            printf("Quantity in Stock: %d\n", current->quantityInStock);
            printf("Reorder Level: %d\n", current->reorderLevel);
            printf("------------------------------------------------------------\n");
            alertCount++;
        }
        current = current->next;
    }

    if (alertCount == 0) {
        printf("No stock alerts to display.\n");
    }
}



int isExpiryDateNear(char* expiryDate) {
    // Get the current date
    time_t now = time(NULL);
    struct tm* currentDate = localtime(&now);

    //  ( DD-MM-YYYY)
    int day, month, year;
    sscanf(expiryDate, "%d-%d-%d", &day, &month, &year);

    // Convert expiration date to time_t
    struct tm expiry = {0};
    expiry.tm_mday = day;      // Day
    expiry.tm_mon = month - 1; // Month (0-based)
    expiry.tm_year = year - 1900; // Year (years since 1900)
    time_t expiryTime = mktime(&expiry);

    // Calculate the difference in seconds between expiration date and current date
    double diff = difftime(expiryTime, now);

    // Convert difference to days
    int daysRemaining = (int)(diff / (60 * 60 * 24));

    // Check if the medication is nearing expiry (within 30 days)
    if (daysRemaining >= 0 && daysRemaining <= 30) {
        return 1; 
    }

    return 0; 
}



void checkExpirationDates() {
    Medication* current = medicationList;
    int alertCount = 0;

    printf("\nExpiration Alerts:\n");
    printf("------------------------------------------------------------\n");

    while (current != NULL) {
        if (isExpiryDateNear(current->expirationDate)) {
            printf("Medication ID: %d\n", current->medicationID);
            printf("Medication Name: %s\n", current->medicationName);
            printf("Batch Number: %d\n", current->batchNumber);
            printf("Expiration Date: %s\n", current->expirationDate);
            printf("------------------------------------------------------------\n");
            alertCount++;
        }
        current = current->next;
    }

    if (alertCount == 0) {
        printf("No expiration alerts to display.\n");
    }
}


int compareDates(char* date1, char* date2) {
    int day1, month1, year1;
    int day2, month2, year2;
    sscanf(date1, "%d-%d-%d", &day1, &month1, &year1);
    sscanf(date2, "%d-%d-%d", &day2, &month2, &year2);

    if (year1 != year2) {
        return year1 - year2;
    }
    if (month1 != month2) {
        return month1 - month2;
    }
    return day1 - day2;
}



Medication* merge(Medication* left, Medication* right) {
    Medication* result = NULL;

    // Base cases
    if (left == NULL) {
        return right;
    }
    if (right == NULL) {
        return left;
    }

    if (compareDates(left->expirationDate, right->expirationDate) <= 0) {
        result = left;
        result->next = merge(left->next, right);
    } else {
        result = right;
        result->next = merge(left, right->next);
    }

    return result;
}


void splitList(Medication* head, Medication** left, Medication** right) {
    Medication* slow = head;
    Medication* fast = head->next;

    while (fast != NULL) {
        fast = fast->next;
        if (fast != NULL) {
            slow = slow->next;
            fast = fast->next;
        }
    }

    *left = head;
    *right = slow->next;
    slow->next = NULL;
}


void mergeSort(Medication** headRef) {
    Medication* head = *headRef;
    Medication* left;
    Medication* right;

    // Base case: if the list is empty or has only one node
    if (head == NULL || head->next == NULL) {
        return;
    }

    // Split the list into two halves
    splitList(head, &left, &right);

    // Recursively sort both halves
    mergeSort(&left);
    mergeSort(&right);

    // Merge the sorted halves
    *headRef = merge(left, right);
}

void sortMedicationsByExpiration() {
    if (medicationList == NULL || medicationList->next == NULL) {
        printf("No medications to sort or only one medication exists.\n");
        return;
    }

    mergeSort(&medicationList);

    printf("Medications sorted by expiration date successfully!\n");

    saveDataToFile();
}


void salesTracking() {
    int medicationID, batchNumber, quantitySold;
    printf("Enter Medication ID: ");
    scanf("%d", &medicationID);
    printf("Enter Batch Number: ");
    scanf("%d", &batchNumber);
    printf("Enter Quantity Sold: ");
    scanf("%d", &quantitySold);

    Medication* current = medicationList;
    while (current != NULL) {
        if (current->medicationID == medicationID && current->batchNumber == batchNumber) {
            if (current->quantityInStock >= quantitySold) {
              
                current->quantityInStock -= quantitySold;
                current->totalSales += quantitySold;
                printf("Sales recorded successfully!\n");

                saveDataToFile();
                return;
            } else {
                printf("Error: Not enough stock available!\n");
                return;
            }
        }
        current = current->next;
    }

    printf("Medication with ID %d and Batch Number %d not found!\n", medicationID, batchNumber);
}



void addSupplier() {
    int medicationID, batchNumber;
    printf("Enter Medication ID to add supplier: ");
    scanf("%d", &medicationID);
    printf("Enter Batch Number: ");
    scanf("%d", &batchNumber);

    Medication* current = medicationList;
    while (current != NULL) {
        if (current->medicationID == medicationID && current->batchNumber == batchNumber) {
            Supplier* newSupplier = (Supplier*)malloc(sizeof(Supplier));
            printf("Enter Supplier ID: ");
            scanf("%d", &newSupplier->supplierID);
            printf("Enter Supplier Name: ");
            scanf("%s", newSupplier->supplierName);
            printf("Enter Quantity Supplied: ");
            scanf("%d", &newSupplier->quantitySupplied);
            printf("Enter Contact Information: ");
            scanf("%s", newSupplier->contactInfo);

            // Add supplier to the medication's supplier list
            newSupplier->next = current->supplierList;
            current->supplierList = newSupplier;

            printf("Supplier added successfully!\n");

           
            saveDataToFile();
            return;
        }
        current = current->next;
    }

    printf("Medication with ID %d and Batch Number %d not found!\n", medicationID, batchNumber);
}



void updateSupplier() {
    int supplierID;
    printf("Enter Supplier ID to update: ");
    scanf("%d", &supplierID);

    Medication* current = medicationList;
    while (current != NULL) {
        Supplier* supplier = current->supplierList;
        while (supplier != NULL) {
            if (supplier->supplierID == supplierID) {
                printf("Enter new Supplier Name: ");
                scanf("%s", supplier->supplierName);
                printf("Enter new Quantity Supplied: ");
                scanf("%d", &supplier->quantitySupplied);
                printf("Enter new Contact Information: ");
                scanf("%s", supplier->contactInfo);

                printf("Supplier details updated successfully!\n");

                saveDataToFile();
                return;
            }
            supplier = supplier->next;
        }
        current = current->next;
    }

    printf("Supplier with ID %d not found!\n", supplierID);
}


void deleteSupplier() {
    int supplierID;
    printf("Enter Supplier ID to delete: ");
    scanf("%d", &supplierID);

    Medication* current = medicationList;
    while (current != NULL) {
        Supplier* supplier = current->supplierList;
        Supplier* prevSupplier = NULL;

        while (supplier != NULL) {
            if (supplier->supplierID == supplierID) {
                if (prevSupplier == NULL) {
                    // If the supplier to delete is the first node
                    current->supplierList = supplier->next;
                } else {
                    // If the supplier to delete is in the middle or end
                    prevSupplier->next = supplier->next;
                }

                free(supplier);
                printf("Supplier deleted successfully!\n");
      
                saveDataToFile();
                return;
            }
            prevSupplier = supplier;
            supplier = supplier->next;
        }
        current = current->next;
    }

    printf("Supplier with ID %d not found!\n", supplierID);
}



void searchSupplier() {
    int supplierID;
    printf("Enter Supplier ID to search: ");
    scanf("%d", &supplierID);

    Medication* current = medicationList;
    while (current != NULL) {
        Supplier* supplier = current->supplierList;
        while (supplier != NULL) {
            if (supplier->supplierID == supplierID) {
                printf("Supplier found!\n");
                printf("Supplier ID: %d\n", supplier->supplierID);
                printf("Supplier Name: %s\n", supplier->supplierName);
                printf("Quantity Supplied: %d\n", supplier->quantitySupplied);
                printf("Contact Info: %s\n", supplier->contactInfo);
                return;
            }
            supplier = supplier->next;
        }
        current = current->next;
    }

    printf("Supplier with ID %d not found!\n", supplierID);
}



void  supplierManagement(){
    int ch;
      printf("enter your choice\n");
      printf("1.add supplier\n");
      printf("2.update supplier\n");
      printf("3.delete supplier\n");
      printf("4.search supplier\n");
      scanf("%d",&ch);
      switch(ch){
        case 1: addSupplier(); break;
        case 2: updateSupplier(); break;
        case 3: deleteSupplier(); break;
        case 4: searchSupplier(); break;
    }
}

void findAllRounderSuppliers() {
    int maxUniqueMedications = 0;
    Supplier* allRounderSupplier = NULL;

    Medication* current = medicationList;
    while (current != NULL) {
        Supplier* supplier = current->supplierList;
        while (supplier != NULL) {
            int uniqueMedications = 0;

            // Count unique medications supplied by this supplier
            Medication* temp = medicationList;
            while (temp != NULL) {
                Supplier* s = temp->supplierList;
                while (s != NULL) {
                    if (s->supplierID == supplier->supplierID) {
                        uniqueMedications++;
                        //break;
                    }
                    s = s->next;
                }
                temp = temp->next;
            }

            // Update all-rounder supplier
            if (uniqueMedications > maxUniqueMedications) {
                maxUniqueMedications = uniqueMedications;
                allRounderSupplier = supplier;
            }

            supplier = supplier->next;
        }
        current = current->next;
    }

    if (allRounderSupplier != NULL) {
        printf("Best All-Rounder Supplier:\n");
        printf("Supplier ID: %d\n", allRounderSupplier->supplierID);
        printf("Supplier Name: %s\n", allRounderSupplier->supplierName);
        printf("Unique Medications Supplied: %d\n", maxUniqueMedications);
    } else {
        printf("No all-rounder suppliers found!\n");
    }
}



void findLargestTurnoverSuppliers() {
    float maxTurnover = 0;
    Supplier* largestTurnoverSupplier = NULL;

    Medication* current = medicationList;
    while (current != NULL) {
        Supplier* supplier = current->supplierList;
        while (supplier != NULL) {
            float turnover = supplier->quantitySupplied * current->pricePerUnit;

            // Update largest turnover supplier
            if (turnover > maxTurnover) {
                maxTurnover = turnover;
                largestTurnoverSupplier = supplier;
            }

            supplier = supplier->next;
        }
        current = current->next;
    }

    if (largestTurnoverSupplier != NULL) {
        printf("Supplier with Largest Turnover:\n");
        printf("Supplier ID: %d\n", largestTurnoverSupplier->supplierID);
        printf("Supplier Name: %s\n", largestTurnoverSupplier->supplierName);
        printf("Turnover: %.2f\n", maxTurnover);
    } else {
        printf("No suppliers found!\n");
    }
}




void printPharmacyData() {
    FILE* file = fopen("pharmacy_data.txt", "r");
    if (file == NULL) {
        printf("Error opening file! No data found.\n");
        return;
    }

    printf("\nPharmacy Inventory Data:\n");
    printf("------------------------------------------------------------\n");

    int medicationFlag = 1;
    while (medicationFlag) {
        int medicationID, batchNumber, quantityInStock, reorderLevel, totalSales;
        char medicationName[50], expirationDate[11];
        float pricePerUnit;

        // Read medication details
        if (fscanf(file, "%d %s %d %d %f %s %d %d",
                   &medicationID, medicationName, &batchNumber, &quantityInStock,
                   &pricePerUnit, expirationDate, &reorderLevel, &totalSales) != 8) {
            medicationFlag = 0; // Stop reading medications
            continue;
        }

        // Print medication details
        printf("Medication ID: %d\n", medicationID);
        printf("Medication Name: %s\n", medicationName);
        printf("Batch Number: %d\n", batchNumber);
        printf("Quantity in Stock: %d\n", quantityInStock);
        printf("Price per Unit: %.2f\n", pricePerUnit);
        printf("Expiration Date: %s\n", expirationDate);
        printf("Reorder Level: %d\n", reorderLevel);
        printf("Total Sales: %d\n", totalSales);
        printf("Suppliers:\n");

        // Read and print supplier details
        int supplierFlag = 1;
        while (supplierFlag) {
            int supplierID, quantitySupplied;
            char supplierName[50], contactInfo[50], buffer[20];

            // Read supplier details
            if (fscanf(file, "%d %s %d %s", &supplierID, supplierName, &quantitySupplied, contactInfo) != 4) {
                supplierFlag = 0; // Stop reading suppliers if data is missing
                continue;
            }

            // Print supplier details
            printf("  Supplier ID: %d\n", supplierID);
            printf("  Supplier Name: %s\n", supplierName);
            printf("  Quantity Supplied: %d\n", quantitySupplied);
            printf("  Contact Info: %s\n", contactInfo);

            // Check for "END_SUPPLIERS" marker
            if (fscanf(file, "%s", buffer) == 1 && strcmp(buffer, "END_SUPPLIERS") == 0) {
                supplierFlag = 0; // Stop reading suppliers
            }
        }

        printf("------------------------------------------------------------\n");
    }

    fclose(file);
}
