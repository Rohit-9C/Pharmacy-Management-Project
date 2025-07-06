#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_MED_NAME 100
#define MAX_SUPPLIER_NAME 100
#define MAX_CONTACT_INFO 50
#define DATE_LEN 20
#define MAX_SUPPLIERS 100
#define MAX_BATCHES 10
#define MAX_MEDICATIONS 100
#define REORDER_MIN 10
#define REORDER_MAX 100

struct supplier
{
    char supplierID[20];
    char supplierName[MAX_SUPPLIER_NAME];
    int suppliedQuantity;
    char contactInfo[MAX_CONTACT_INFO];
    double turnover;
};

struct Batch
{
    char batchNumber[50];
    int quantityInStock;
    float pricePerUnit;
    char expirationDate[DATE_LEN];
    int totalSales;
};

struct Medication
{
    char medicationID[20];
    char medicationName[MAX_MED_NAME];
    int batchCount;
    struct Batch batches[MAX_BATCHES];
    int supplierCount;
    struct supplier suppliers[100];
    int reorderLevel;
};

struct Medication inventory[MAX_MEDICATIONS];
int medicationCount = 0;
struct supplier supplierList[100];
int supplierCount = 0;

void addMedication()
{
    FILE *file = fopen("inventoryR.dat", "ab"); 
    if (file == NULL)
    {
        printf("Error opening file!\n");
        return;
    }

    struct Medication newMed;
    printf("Enter Medication ID: ");
    scanf("%s", newMed.medicationID);
    printf("Enter Medication Name: ");
    scanf("%s", newMed.medicationName);
    printf("Enter Reorder Level (10-100): ");
    scanf("%d", &newMed.reorderLevel);

        if (newMed.reorderLevel < REORDER_MIN || newMed.reorderLevel > REORDER_MAX) {
            printf("Invalid reorder level.\n");
            return;
    }

    // Add batches
    printf("Enter number of batches: ");
    scanf("%d", &newMed.batchCount);
    for (int i = 0; i < newMed.batchCount; i++)
    {
        printf("Enter Batch Number: ");
        scanf("%s", newMed.batches[i].batchNumber);
        printf("Enter Quantity in Stock: ");
        scanf("%d", &newMed.batches[i].quantityInStock);
        printf("Enter Price per Unit: ");
        scanf("%f", &newMed.batches[i].pricePerUnit);
        printf("Enter Expiration Date (DD-MM-YYYY): ");
        scanf("%s", newMed.batches[i].expirationDate);
        newMed.batches[i].totalSales = 0;
    }

    // Add supplier
    printf("Enter number of suppliers: ");
    scanf("%d", &newMed.supplierCount);
    for (int i = 0; i < newMed.supplierCount; i++)
    {
        printf("Enter Supplier Id: ");
        scanf("%s", newMed.suppliers[i].supplierID);
        printf("enter supplier name: ");
        scanf("%s", newMed.suppliers[i].supplierName);
        printf("enter supplier contact information: ");
        scanf("%s", newMed.suppliers[i].contactInfo);
        printf("enter supplied Quantity : ");
        scanf("%d", &newMed.suppliers[i].suppliedQuantity);
        newMed.suppliers[i].turnover = 0.0;
    }

    inventory[medicationCount++] = newMed;

    fwrite(&newMed, sizeof(struct Medication), 1, file);

    fclose(file);
    printf("Medication added successfully!\n");
}

void updateMedication()
{
    struct Medication med;
    FILE *file = fopen("inventoryR.dat", "rb+"); 

    if (file == NULL)
    {
        printf("Error opening file!\n");
        return;
    }

    char searchID[20];
    printf("Enter medication ID: ");
    scanf("%s", searchID);

    while (fread(&med, sizeof(struct Medication), 1, file))
    {
        if (strcmp(med.medicationID, searchID) == 0)
        {
            printf("Medication found: %s\n", med.medicationName);
            printf("Available Batches:\n");
            for (int i = 0; i < med.batchCount; i++)
            {
                printf("Batch %d: %s\n", i + 1, med.batches[i].batchNumber);
            }

            int batchIndex;
            printf("Enter the index of the batch to update (1 to %d): ", med.batchCount);
            scanf("%d", &batchIndex);
            batchIndex--; 

            printf("Enter new Quantity in Stock: ");
            scanf("%d", &med.batches[batchIndex].quantityInStock);
            printf("Enter new Price per Unit: ");
            scanf("%f", &med.batches[batchIndex].pricePerUnit);

                                                                  
            fseek(file, -sizeof(struct Medication), SEEK_CUR); 
            fwrite(&med, sizeof(struct Medication), 1, file);
            printf("Batch updated successfully!\n");
            return; 
        }
    }
    
    if (feof(file))
    {
        printf("Medication not found!\n");
    }

   
    fclose(file);
}

void deleteMedication()
{
    struct Medication med;
    char searchID[20];
    printf("Enter medication ID: ");
    scanf("%s", searchID);

    FILE *file = fopen("inventoryR.dat", "rb"); 
    FILE *tempFile = fopen("temp.dat", "wb");   

    if (file == NULL || tempFile == NULL)
    {
        printf("Error opening file!\n");
        return;
    }

    int found = 0;

    while (fread(&med, sizeof(struct Medication), 1, file))
    {
        if (strcmp(med.medicationID, searchID) != 0)
        {
            fwrite(&med, sizeof(struct Medication), 1, tempFile); 
        }
        else
        {
            found = 1;
        }
    }

   
    fclose(file);
    fclose(tempFile);

    remove("inventoryR.dat");
    rename("temp.dat", "inventoryR.dat");

    if (found)
    {
        printf("Medication deleted successfully!\n");
    }

    else
    {
        printf("Medication not found!\n");
    }
}

void searchMedication()
{
    char medId[20];
    printf("Enter medication ID to search: ");
    scanf("%s", medId);

    struct Medication med;
    FILE *file = fopen("inventoryR.dat", "rb"); 

    if (file == NULL)
    {
        printf("Error opening file or no records found!\n");
        return;
    }

    int found = 0; 

    while (fread(&med, sizeof(struct Medication), 1, file))
    {
        if (strcmp(med.medicationID, medId) == 0)
        {
            printf("Medication Found.\n");
            printf("Name: %s\n", med.medicationName);
            printf("Total batches : %d \n",med.batchCount);
            found = 1; 
            return;     
        }
    }

    if (!found)
    {
        printf("Medication not found.\n");
    }

    fclose(file);
}

void checkStockAlert()
{
    FILE *file = fopen("inventoryR.dat", "rb");

    if (file == NULL)
    {
        printf("Error opening file!\n");
        return;
    }

    struct Medication med;
    int found = 0; 

    while (fread(&med, sizeof(struct Medication), 1, file))
    {
        for (int j = 0; j < med.batchCount; j++)
        {
            if (med.batches[j].quantityInStock < med.reorderLevel)
            {
                printf("Stock Alert: Medication %s - Batch %s is below reorder level.\n",
                       med.medicationID, med.batches[j].batchNumber);
                found = 1; 
            }
        }
    }

    if (!found)
    {
        printf("All medications are above the reorder level.\n");
    }

    fclose(file);
}

int isDateApproachingExpiry(char *expiryDate)
{
    // Convert the expiry date string to a time structure
    struct tm expiry = {0};    // Initialize to zero
    sscanf(expiryDate, "%d-%d-%d", &expiry.tm_mday, &expiry.tm_mon, &expiry.tm_year);
    expiry.tm_mon -= 1;       // tm_mon is 0-based
    expiry.tm_year -= 1900;   // Adjust correctly to years since 1900

    time_t now = time(NULL);
    struct tm *current = localtime(&now);

   
    double seconds = difftime(mktime(&expiry), mktime(current));
    int days = seconds / (60 * 60 * 24);

    return (days <= 30 && days >= 0); 
}

void checkExpirationDates()
{
    FILE *file = fopen("inventoryR.dat", "rb"); 

    if (file == NULL)
    {
        printf("Error opening file!\n");
        return;
    }

    struct Medication med;
    int found = 0; 
    while (fread(&med, sizeof(struct Medication), 1, file))
    {
        for (int j = 0; j < med.batchCount; j++)
        {
           
            if (isDateApproachingExpiry(med.batches[j].expirationDate))
            {
                printf("Expiration Alert: Medication %s - Batch %s is approaching expiration.\n",
                       med.medicationID, med.batches[j].batchNumber);
                printf(" expiration date is : %s\n", med.batches[j].expirationDate);
                found = 1; 
            }
        }
    }

    if (!found)
    {
        printf("No medications are approaching expiration.\n");
    }

    
    fclose(file);
}

void trackSales()
{
    char medID[20], batchNumber[20];
    int quantitySold;

    printf("Enter Medication ID: ");
    scanf("%s", medID);

   
    printf("Enter Batch Number: ");
    scanf("%s", batchNumber);

   
    printf("Enter Quantity Sold: ");
    scanf("%d", &quantitySold);

   
    FILE *file = fopen("inventoryR.dat", "rb+");
    if (file == NULL)
    {
        printf("Error opening inventory file!\n");
        return;
    }

    struct Medication med;
    int foundMedication = 0;
    int foundBatch = 0;

   
    while (fread(&med, sizeof(struct Medication), 1, file))
    {
       
        if (strcmp(med.medicationID, medID) == 0)
        {
            foundMedication = 1;

          
            for (int j = 0; j < med.batchCount; j++)
            {
                if (strcmp(med.batches[j].batchNumber, batchNumber) == 0)
                {
                    foundBatch = 1;

                    
                    if (quantitySold > med.batches[j].quantityInStock)
                    {
                        printf("Error: Quantity sold exceeds quantity in stock!\n");
                        fclose(file);
                        return;
                    }

                
                    med.batches[j].totalSales += quantitySold;
                    med.batches[j].quantityInStock -= quantitySold;

                
                    fseek(file, -sizeof(struct Medication), SEEK_CUR);
                    fwrite(&med, sizeof(struct Medication), 1, file);
                    printf("Sales recorded successfully!\n");
                    return; 
                }
            }
            return; 
        }
    }

    if (!foundMedication)
    {
        printf("Error: Medication not found!\n");
    }
    else if (!foundBatch)
    {
        printf("Error: Batch not found!\n");
    }


    fclose(file);
}


int convertDateToInt(char *date)
{
    int day, month, year;
    sscanf(date, "%d-%d-%d", &day, &month, &year);
    return year * 10000 + month * 100 + day;
}

  // Quick Sort partition function
int partition(struct Medication *meds, int low, int high)
{
    int pivotDate = convertDateToInt(meds[high].batches[0].expirationDate); 
    int i = (low - 1);                                                      
    for (int j = low; j < high; j++)
    {
        if (convertDateToInt(meds[j].batches[0].expirationDate) < pivotDate)
        {
            i++;
            struct Medication temp = meds[i];
            meds[i] = meds[j];
            meds[j] = temp;
        }
    }
    struct Medication temp = meds[i + 1];
    meds[i + 1] = meds[high];
    meds[high] = temp;
    return (i + 1);
}

void quickSort(struct Medication *meds, int low, int high)
{
    if (low < high)
    {
        int pi = partition(meds, low, high);
        quickSort(meds, low, pi - 1);
        quickSort(meds, pi + 1, high);
    }
}


void sortMedicationbyExpiry()
{
    struct Medication inventory[MAX_MEDICATIONS];
    int medicationCount = 0;

   
    FILE *file = fopen("inventoryR.dat", "rb");
    if (file == NULL)
    {
        printf("Error opening inventory file!\n");
        return;
    }

   
    while (fread(&inventory[medicationCount], sizeof(struct Medication), 1, file))
    {
        medicationCount++;
    }
    fclose(file);

  
    quickSort(inventory, 0, medicationCount - 1);

    file = fopen("inventoryR.dat", "wb");
    if (file == NULL)
    {
        printf("Error opening inventory file for writing!\n");
        return;
    }

   
    for (int i = 0; i < medicationCount; i++)
    {
        fwrite(&inventory[i], sizeof(struct Medication), 1, file);
    }

    fclose(file); 
    printf("Medications sorted by expiration date successfully!\n");
}


void addSupplier()
{
    char medID[20];
    struct supplier newSupplier;

 
    printf("Enter Medication ID to which the supplier will be added: ");
    scanf("%s", medID);

   
    printf("Enter Supplier ID: ");
    scanf("%s", newSupplier.supplierID);
    printf("Enter Supplier NAME: ");
    scanf("%s", newSupplier.supplierName);
    printf("Enter supplier contact info: ");
    scanf("%s", newSupplier.contactInfo);
    newSupplier.suppliedQuantity = 0;
    newSupplier.turnover = 0.0;

   
    FILE *file = fopen("inventoryR.dat", "rb+");
    if (file == NULL)
    {
        printf("Error opening inventory file for writing!\n");
        return; 
    }

    struct Medication med;
    int foundMedication = 0;

  
    while (fread(&med, sizeof(struct Medication), 1, file))
    {
      
        if (strcmp(med.medicationID, medID) == 0)
        {
            foundMedication = 1;

        
            if (med.supplierCount < 100) 
            {
                med.suppliers[med.supplierCount++] = newSupplier;               
                fseek(file, -sizeof(struct Medication), SEEK_CUR);
                fwrite(&med, sizeof(struct Medication), 1, file);
                printf("Supplier added successfully to Medication ID %s!\n", medID);
            }
            else
            {
                printf("Cannot add more suppliers to this medication.\n");
            }
            break; 
        }
    }

    if (!foundMedication)
    {
        printf("Medication not found.\n");
    }

   
    fclose(file);
}

void searchSupplier()
{
    char supplierID[20];
    printf("Enter Supplier ID to search: ");
    scanf("%s", supplierID);

   
    FILE *file = fopen("inventoryR.dat", "rb");
    if (file == NULL)
    {
        printf("Error opening inventory file for reading!\n");
        return; 
    }

    struct Medication med;
    int found = 0;

 
    while (fread(&med, sizeof(struct Medication), 1, file))
    {
       
        for (int i = 0; i < med.supplierCount; i++)
        {
            if (strcmp(med.suppliers[i].supplierID, supplierID) == 0)
            {
                printf("Supplier found:\n");
                printf("ID: %s\n, Name: %s\n, Contact Info: %s\n, Quantity Supplied: %d\n, Turnover: %.2f\n",
                       med.suppliers[i].supplierID, med.suppliers[i].supplierName,
                       med.suppliers[i].contactInfo, med.suppliers[i].suppliedQuantity,
                       med.suppliers[i].turnover);
                found = 1; 
                break;  
            }
        }
        if (found)
            break; 
    }

    if (!found)
    {
        printf("Supplier not found.\n");
    }

    fclose(file); 
}

void manageSuppliers()
{
    int choice;
    printf("Supplier Management\n");
    printf("1. Add Supplier\n");
    printf("2. Search Supplier\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    switch (choice)
    {
    case 1:
        addSupplier();
        break;
    case 2:
        searchSupplier();
        break;
    default:
        printf("Invalid choice.\n");
        break;
    }
}

void findTopAllrounderSuppliers()
{
  
    int uniqueMedCount[MAX_SUPPLIERS] = {0};

    FILE *file = fopen("inventoryR.dat", "rb");
    if (file == NULL)
    {
        printf("Error opening inventory file for reading!\n");
        return;
    }

  
    struct Medication med;
    while (fread(&med, sizeof(struct Medication), 1, file))
    {
        for (int j = 0; j < med.supplierCount; j++)
        {
            for (int k = 0; k < supplierCount; k++)
            {
                if (strcmp(med.suppliers[j].supplierID, supplierList[k].supplierID) == 0)
                {
                    uniqueMedCount[k]++;
                }
            }
        }
    }
    fclose(file); 

   
    for (int i = 0; i < supplierCount - 1; i++)
    {
        for (int j = i + 1; j < supplierCount; j++)
        {
            if (uniqueMedCount[i] < uniqueMedCount[j])
            {
                int tempCount = uniqueMedCount[i];
                uniqueMedCount[i] = uniqueMedCount[j];
                uniqueMedCount[j] = tempCount;

                struct supplier temp = supplierList[i];
                supplierList[i] = supplierList[j];
                supplierList[j] = temp;
            }
        }
    }

   
    printf("Top All Rounder Suppliers: \n");
    for (int i = 0; i < supplierCount && i < 10; i++)
    {
        printf("%s - %s (Unique Medications: %d)\n",
               supplierList[i].supplierID, supplierList[i].supplierName, uniqueMedCount[i]);
    }
}

void findSupplierWithlargestTurnover()
{
   
    for (int i = 0; i < supplierCount; i++)
    {
        supplierList[i].turnover = 0.0;
    }

  
    FILE *file = fopen("inventoryR.dat", "rb");
    if (file == NULL)
    {
        printf("Error opening inventory file for reading!\n");
        return; 
    }

   
    struct Medication med;
    while (fread(&med, sizeof(struct Medication), 1, file))
    {
        for (int j = 0; j < med.batchCount; j++)
        {
            double totalValue = med.batches[j].pricePerUnit * med.batches[j].quantityInStock;

            for (int k = 0; k < med.supplierCount; k++)
            {
                for (int s = 0; s < supplierCount; s++)
                {
                    if (strcmp(med.suppliers[k].supplierID, supplierList[s].supplierID) == 0)
                    {
                        supplierList[s].turnover += totalValue;
                    }
                }
            }
        }
    }

    fclose(file); 

    for (int i = 0; i < supplierCount - 1; i++)
    {
        for (int j = i + 1; j < supplierCount; j++)
        {
            if (supplierList[i].turnover < supplierList[j].turnover)
            {
                struct supplier temp = supplierList[i];
                supplierList[i] = supplierList[j];
                supplierList[j] = temp;
            }
        }
    }

    printf("Top  Suppliers with Largest Turnover: \n");
    for (int i = 0; i < supplierCount && i < 10; i++)
    {
        printf("%s - %s (Turnover: %.2f)\n",
               supplierList[i].supplierID, supplierList[i].supplierName, supplierList[i].turnover);
    }
}
void readMedications()
{
    struct Medication med;
    FILE *file = fopen("inventoryR.dat", "rb"); 

    if (file == NULL)
    {
        printf("Error opening file or no records found!\n");
        return;
    }

    printf("\nMedication Records:\n");
    printf("------------------------------------------------\n");
    while (fread(&med, sizeof(struct Medication), 1, file))
    {
        printf(" Medication ID: %s\n", med.medicationID);
        printf(" Medication Name: %s\n", med.medicationName);

        for (int i = 0; i < med.batchCount; i++)
        {
            printf("Batch: %d \n", i + 1);
            printf("Batch Number: %s \n", med.batches[i].batchNumber);
            printf(" Batch quantity: %d \n", med.batches[i].quantityInStock);
            printf(" Batch price: %lf \n", med.batches[i].pricePerUnit);
            printf(" Exipration date of Batch : %s \n", med.batches[i].expirationDate);
        }

        for (int i = 0; i < med.supplierCount; i++)
        {
            printf("Supplier: %d \n", i + 1);
            printf(" Supplier ID: %s \n", med.suppliers[i].supplierID);
            printf(" Supplier Name: %s \n", med.suppliers[i].supplierName);
            printf(" Contact Info: %s\n", med.suppliers[i].contactInfo);
            printf(" Supplied Quantity: %d \n", med.suppliers[i].suppliedQuantity);
        }
        printf("------------------------------------------------\n");
    }

   
    fclose(file);
}

int main()
{
    int select;

    while (1)
    {

        printf("   Pharmacy Inventory Managemant system  \n");
        printf("1. Add New Medication \n");
        printf("2. Update Medication Details \n");
        printf("3. Delete Medication  \n");
        printf("4. Search Medication \n");
        printf("5. Stock Alerts \n");
        printf("6. Check Expiration Dates \n");
        printf("7. Track Sale \n");
        printf("8. Sort Medications by Expiratian date  \n");
        printf("9.  Manage Suppliers \n");
        printf("10. Find Top All-Rounder Suppliers \n");
        printf("11. Find Suppliers with largest Turnover \n");
        printf("12. read data \n");
        printf("13. Exit \n");
        printf("enter your choice:");
        scanf("%d", &select);

        switch (select)
        {
        case 1:
            addMedication();
            break;
        case 2:
            updateMedication();
            break;
        case 3:
            deleteMedication();
            break;
        case 4:
            searchMedication();
            break;
        case 5:
            checkStockAlert();
            break;
        case 6:
            checkExpirationDates();
            break;
        case 7:
            trackSales();
            break;
        case 8:
            sortMedicationbyExpiry();
            break;
        case 9:
            manageSuppliers();
            break;
        case 10:
            findTopAllrounderSuppliers();
            break;
        case 11:
            findSupplierWithlargestTurnover();
            break;
        case 12:
            readMedications();
            break;
        case 13:
            return 0;
        default:
            printf("enter valid choice \n");
            break;
        }
    }
}