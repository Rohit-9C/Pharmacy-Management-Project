#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_SUPPLIERS 10
#define REORDER_MIN 10
#define REORDER_MAX 100
#define EXPIRY_ALERT_DAYS 30
#define ORDER 3  // Order of the B+ tree(assumption: number of key)

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
} Medication;

// B+ Tree Node Structure
typedef struct BPlusTreeNode {
    bool is_leaf;
    int num_keys;
    int keys[ORDER];
    struct BPlusTreeNode* children[ORDER + 1];
    Medication* data[ORDER];  // Only for leaf nodes
    struct BPlusTreeNode* next;  // For linking leaf nodes
} BPlusTreeNode;

BPlusTreeNode* root = NULL;

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
void printTree(BPlusTreeNode* node, int level);
BPlusTreeNode* findParent(BPlusTreeNode* cursor, BPlusTreeNode* child) ;

// B+ Tree operations
BPlusTreeNode* createNode(bool is_leaf);
void insert(int key, Medication* medication);
void insertInternal(int key, BPlusTreeNode* cursor, BPlusTreeNode* child);
Medication* search(int key);
void traverse(BPlusTreeNode* cursor);
void deleteKey(int key);
void deleteInternal(int key, BPlusTreeNode* cursor, BPlusTreeNode* child);


int main() {
    int choice;
    loadDataFromFile(); 

    while (1) {
        printf("\nPharmacy Inventory Management System\n");
        printf("1. Add New Medication\n");
        printf("2. Update Medication Details\n");
        printf("3. Print Pharmacy Data\n");
        printf("4. Delete Medication\n");
        printf("5. Search Medication\n");
        printf("6. Stock Alert of Medication\n");
        printf("7. Check Expiration Dates\n");
        printf("8. Sort Medication by Expiration Date\n");
        printf("9. Sales Tracking\n");
        printf("10. Supplier Management\n");
        printf("11. Find All-rounder Suppliers\n");
        printf("12. Find Supplier with Largest Turn-over\n");
        printf("13. Print B+ Tree Structure\n");
        printf("14. Exit\n");
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
            case 13: printTree(root, 0); break;
            case 14: return 0;
            default: printf("Invalid choice! Please try again.\n");
        }
    }
    return 0;
}

// B+ Tree Implementation

BPlusTreeNode* createNode(bool is_leaf) {
    BPlusTreeNode* newNode = (BPlusTreeNode*)malloc(sizeof(BPlusTreeNode));
    newNode->is_leaf = is_leaf;
    newNode->num_keys = 0;
    newNode->next = NULL;
    for (int i = 0; i < ORDER + 1; i++) {
        newNode->children[i] = NULL; 
        if (i < ORDER) {
            newNode->keys[i] = 0;
            newNode->data[i] = NULL;
        }
    }
    return newNode;
}

void insert(int key, Medication* medication) {
    if (root == NULL) {
        root = createNode(true);
        root->keys[0] = key;
        root->data[0] = medication;
        root->num_keys = 1;
        return;
    }

    BPlusTreeNode* cursor = root;
    BPlusTreeNode* parent = NULL;

    while (!cursor->is_leaf) {
        parent = cursor;
        for (int i = 0; i < cursor->num_keys; i++) {
            if (key < cursor->keys[i]) {
                cursor = cursor->children[i];
                break;
            }
            if (i == cursor->num_keys - 1) {
                cursor = cursor->children[i + 1];
                break;
            }
        }
    }

    if (cursor->num_keys < ORDER) {
        int i = 0;
        while (key > cursor->keys[i] && i < cursor->num_keys) i++;
        
        for (int j = cursor->num_keys; j > i; j--) {
            cursor->keys[j] = cursor->keys[j - 1];
            cursor->data[j] = cursor->data[j - 1];
        }
        
        cursor->keys[i] = key;
        cursor->data[i] = medication;
        cursor->num_keys++;
        cursor->children[cursor->num_keys] = cursor->children[cursor->num_keys - 1];
        cursor->children[cursor->num_keys - 1] = NULL;
    } else {
        BPlusTreeNode* newLeaf = createNode(true);
        Medication* virtualData[ORDER + 1];
        int virtualKeys[ORDER + 1];
        
        for (int i = 0; i < ORDER; i++) {
            virtualKeys[i] = cursor->keys[i];
            virtualData[i] = cursor->data[i];
        }
        
        int i = 0, j;
        while (key > virtualKeys[i] && i < ORDER) i++;
        
        for (int j = ORDER; j > i; j--) {
            virtualKeys[j] = virtualKeys[j - 1];
            virtualData[j] = virtualData[j - 1];
        }
        
        virtualKeys[i] = key;
        virtualData[i] = medication;
        
        newLeaf->next = cursor->next;
        cursor->next = newLeaf;
        cursor->num_keys = (ORDER + 1) / 2;
        newLeaf->num_keys = ORDER + 1 - (ORDER + 1) / 2;
        
        for (i = 0; i < cursor->num_keys; i++) {
            cursor->keys[i] = virtualKeys[i];
            cursor->data[i] = virtualData[i];
        }
        
        for (i = 0, j = cursor->num_keys; i < newLeaf->num_keys; i++, j++) {
            newLeaf->keys[i] = virtualKeys[j];
            newLeaf->data[i] = virtualData[j];
        }
        
        if (cursor == root) {
            BPlusTreeNode* newRoot = createNode(false);
            newRoot->keys[0] = newLeaf->keys[0];
            newRoot->children[0] = cursor;
            newRoot->children[1] = newLeaf;
            newRoot->num_keys = 1;
            root = newRoot;
        } else {
            insertInternal(newLeaf->keys[0], parent, newLeaf);
        }
    }
}

void insertInternal(int key, BPlusTreeNode* cursor, BPlusTreeNode* child) {
    if (cursor->num_keys < ORDER) {
        int i = 0;
        while (key > cursor->keys[i] && i < cursor->num_keys) i++;
        
        for (int j = cursor->num_keys; j > i; j--) {
            cursor->keys[j] = cursor->keys[j - 1];
        }
        
        for (int j = cursor->num_keys + 1; j > i + 1; j--) {
            cursor->children[j] = cursor->children[j - 1];
        }
        
        cursor->keys[i] = key;
        cursor->num_keys++;
        cursor->children[i + 1] = child;
    } else {
        BPlusTreeNode* newInternal = createNode(false);
        int virtualKeys[ORDER + 1];
        BPlusTreeNode* virtualChildren[ORDER + 2];
        
        for (int i = 0; i < ORDER; i++) {
            virtualKeys[i] = cursor->keys[i];
        }
        
        for (int i = 0; i < ORDER + 1; i++) {
            virtualChildren[i] = cursor->children[i];
        }
        
        int i = 0, j;
        while (key > virtualKeys[i] && i < ORDER) i++;
        
        for (int j = ORDER; j > i; j--) {
            virtualKeys[j] = virtualKeys[j - 1];
        }
        
        virtualKeys[i] = key;
        
        for (int j = ORDER + 1; j > i + 1; j--) {
            virtualChildren[j] = virtualChildren[j - 1];
        }
        
        virtualChildren[i + 1] = child;
        
        cursor->num_keys = (ORDER + 1) / 2;
        newInternal->num_keys = ORDER - (ORDER + 1) / 2;
        
        for (i = 0, j = cursor->num_keys + 1; i < newInternal->num_keys; i++, j++) {
            newInternal->keys[i] = virtualKeys[j];
        }
        
        for (i = 0, j = cursor->num_keys + 1; i < newInternal->num_keys + 1; i++, j++) {
            newInternal->children[i] = virtualChildren[j];
        }
        
        if (cursor == root) {
            BPlusTreeNode* newRoot = createNode(false);
            newRoot->keys[0] = virtualKeys[cursor->num_keys];
            newRoot->children[0] = cursor;
            newRoot->children[1] = newInternal;
            newRoot->num_keys = 1;
            root = newRoot;
        } else {
            insertInternal(virtualKeys[cursor->num_keys], findParent(root, cursor), newInternal);
        }
    }
}


BPlusTreeNode* findParent(BPlusTreeNode* cursor, BPlusTreeNode* child) {
    BPlusTreeNode* parent = NULL;
    
    if (cursor->is_leaf || cursor->children[0]->is_leaf) {
        return NULL;
    }
    
    for (int i = 0; i < cursor->num_keys + 1; i++) {
        if (cursor->children[i] == child) {
            parent = cursor;
            return parent;
        } else {
            parent = findParent(cursor->children[i], child);
            if (parent != NULL) return parent;
        }
    }
    
    return parent;
}


Medication* search(int key) {
    if (root == NULL) {
        return NULL;
    }
    
    BPlusTreeNode* cursor = root;
    while (!cursor->is_leaf) {
        for (int i = 0; i < cursor->num_keys; i++) {
            if (key < cursor->keys[i]) {
                cursor = cursor->children[i];
                break;
            }
            if (i == cursor->num_keys - 1) {
                cursor = cursor->children[i + 1];
                break;
            }
        }
    }
    
    for (int i = 0; i < cursor->num_keys; i++) {
        if (cursor->keys[i] == key) {
            return cursor->data[i];
        }
    }
    
    return NULL;
}


void traverse(BPlusTreeNode* cursor) {
    if (cursor != NULL) {
        if (cursor->is_leaf) {
            for (int i = 0; i < cursor->num_keys; i++) {
                printf("%d ", cursor->keys[i]);
            }
            printf("\n");
        } else {
            for (int i = 0; i < cursor->num_keys + 1; i++) {
                traverse(cursor->children[i]);
            }
        }
    }
}


void deleteKey(int key) {
    if (root == NULL) {
        printf("Tree is empty\n");
        return;
    }
    
    BPlusTreeNode* cursor = root;
    BPlusTreeNode* parent = NULL;
    int leftSibling, rightSibling;
    
    while (!cursor->is_leaf) {
        for (int i = 0; i < cursor->num_keys; i++) {
            parent = cursor;
            leftSibling = i - 1;
            rightSibling = i + 1;
            
            if (key < cursor->keys[i]) {
                cursor = cursor->children[i];
                break;
            }
            if (i == cursor->num_keys - 1) {
                leftSibling = i;
                rightSibling = i + 2;
                cursor = cursor->children[i + 1];
                break;
            }
        }
    }
    
    bool found = false;
    int pos;
    for (pos = 0; pos < cursor->num_keys; pos++) {
        if (cursor->keys[pos] == key) {
            found = true;
            break;
        }
    }
    
    if (!found) {
        printf("Key not found\n");
        return;
    }
    
    // Free medication and supplier data
    if (cursor->data[pos] != NULL) {
        Supplier* supplier = cursor->data[pos]->supplierList;
        while (supplier != NULL) {
            Supplier* temp = supplier;
            supplier = supplier->next;
            free(temp);
        }
        free(cursor->data[pos]);
    }
    
    for (int i = pos; i < cursor->num_keys - 1; i++) {
        cursor->keys[i] = cursor->keys[i + 1];
        cursor->data[i] = cursor->data[i + 1];
    }
    cursor->num_keys--;
    
    if (cursor == root) {
        if (cursor->num_keys == 0) {
            free(cursor);
            root = NULL;
        }
        return;
    }
    
    if (cursor->num_keys >= (ORDER + 1) / 2 - 1) {
        return;
    }
    
    // Try to borrow from left sibling
    if (leftSibling >= 0) {
        BPlusTreeNode* leftNode = parent->children[leftSibling];
        if (leftNode->num_keys >= (ORDER + 1) / 2) {
            for (int i = cursor->num_keys; i > 0; i--) {
                cursor->keys[i] = cursor->keys[i - 1];
                cursor->data[i] = cursor->data[i - 1];
            }
            
            cursor->keys[0] = leftNode->keys[leftNode->num_keys - 1];
            cursor->data[0] = leftNode->data[leftNode->num_keys - 1];
            cursor->num_keys++;
            leftNode->num_keys--;
            
            parent->keys[leftSibling] = cursor->keys[0];
            return;
        }
    }
    

    // Try to borrow from right sibling
    if (rightSibling <= parent->num_keys) {
        BPlusTreeNode* rightNode = parent->children[rightSibling];
        if (rightNode->num_keys >= (ORDER + 1) / 2) {
            cursor->keys[cursor->num_keys] = rightNode->keys[0];
            cursor->data[cursor->num_keys] = rightNode->data[0];
            cursor->num_keys++;
            
            for (int i = 0; i < rightNode->num_keys - 1; i++) {
                rightNode->keys[i] = rightNode->keys[i + 1];
                rightNode->data[i] = rightNode->data[i + 1];
            }
            rightNode->num_keys--;
            
            parent->keys[rightSibling - 1] = rightNode->keys[0];
            return;
        }
    }
    
    // Merge with left sibling
    if (leftSibling >= 0) {
        BPlusTreeNode* leftNode = parent->children[leftSibling];
        
        for (int i = leftNode->num_keys, j = 0; j < cursor->num_keys; i++, j++) {
            leftNode->keys[i] = cursor->keys[j];
            leftNode->data[i] = cursor->data[j];
        }
        
        leftNode->num_keys += cursor->num_keys;
        leftNode->next = cursor->next;
        
        deleteInternal(parent->keys[leftSibling], parent, cursor);
        free(cursor);
    } 
    // Merge with right sibling
    else if (rightSibling <= parent->num_keys) {
        BPlusTreeNode* rightNode = parent->children[rightSibling];
        
        for (int i = cursor->num_keys, j = 0; j < rightNode->num_keys; i++, j++) {
            cursor->keys[i] = rightNode->keys[j];
            cursor->data[i] = rightNode->data[j];
        }
        
        cursor->num_keys += rightNode->num_keys;
        cursor->next = rightNode->next;
        
        deleteInternal(parent->keys[rightSibling - 1], parent, rightNode);
        free(rightNode);
    }
}



void deleteInternal(int key, BPlusTreeNode* cursor, BPlusTreeNode* child) {
    if (cursor == root && cursor->num_keys == 1) {
        if (cursor->children[0] == child) {
            free(child);
            root = cursor->children[1];
            free(cursor);
            return;
        } else if (cursor->children[1] == child) {
            free(child);
            root = cursor->children[0];
            free(cursor);
            return;
        }
    }
    
    int pos;
    for (pos = 0; pos < cursor->num_keys; pos++) {
        if (cursor->keys[pos] == key) {
            break;
        }
    }
    
    for (int i = pos; i < cursor->num_keys - 1; i++) {
        cursor->keys[i] = cursor->keys[i + 1];
    }
    
    for (pos = 0; pos < cursor->num_keys + 1; pos++) {
        if (cursor->children[pos] == child) {
            break;
        }
    }
    
    for (int i = pos; i < cursor->num_keys; i++) {
        cursor->children[i] = cursor->children[i + 1];
    }
    
    cursor->num_keys--;
    
    if (cursor->num_keys >= (ORDER + 1) / 2 - 1) {
        return;
    }
    
    if (cursor == root) {
        return;
    }
    
    BPlusTreeNode* parent = findParent(root, cursor);
    int leftSibling, rightSibling;
    
    for (pos = 0; pos < parent->num_keys + 1; pos++) {
        if (parent->children[pos] == cursor) {
            leftSibling = pos - 1;
            rightSibling = pos + 1;
            break;
        }
    }
    
    // Try to borrow from left sibling
    if (leftSibling >= 0) {
        BPlusTreeNode* leftNode = parent->children[leftSibling];
        if (leftNode->num_keys >= (ORDER + 1) / 2) {
            for (int i = cursor->num_keys; i > 0; i--) {
                cursor->keys[i] = cursor->keys[i - 1];
            }
            
            for (int i = cursor->num_keys + 1; i > 0; i--) {
                cursor->children[i] = cursor->children[i - 1];
            }
            
            cursor->keys[0] = parent->keys[leftSibling];
            parent->keys[leftSibling] = leftNode->keys[leftNode->num_keys - 1];
            
            cursor->children[0] = leftNode->children[leftNode->num_keys];
            cursor->num_keys++;
            leftNode->num_keys--;
            return;
        }
    }
    
    // Try to borrow from right sibling
    if (rightSibling <= parent->num_keys) {
        BPlusTreeNode* rightNode = parent->children[rightSibling];
        if (rightNode->num_keys >= (ORDER + 1) / 2) {
            cursor->keys[cursor->num_keys] = parent->keys[pos];
            parent->keys[pos] = rightNode->keys[0];
            
            for (int i = 0; i < rightNode->num_keys - 1; i++) {
                rightNode->keys[i] = rightNode->keys[i + 1];
            }
            
            cursor->children[cursor->num_keys + 1] = rightNode->children[0];
            
            for (int i = 0; i < rightNode->num_keys; i++) {
                rightNode->children[i] = rightNode->children[i + 1];
            }
            
            cursor->num_keys++;
            rightNode->num_keys--;
            return;
        }
    }
    
    // Merge with left sibling
    if (leftSibling >= 0) {
        BPlusTreeNode* leftNode = parent->children[leftSibling];
        
        leftNode->keys[leftNode->num_keys] = parent->keys[leftSibling];
        
        for (int i = leftNode->num_keys + 1, j = 0; j < cursor->num_keys; j++) {
            leftNode->keys[i] = cursor->keys[j];
        }
        
        for (int i = leftNode->num_keys + 1, j = 0; j < cursor->num_keys + 1; j++) {
            leftNode->children[i] = cursor->children[j];
            cursor->children[j] = NULL;
        }
        
        leftNode->num_keys += cursor->num_keys + 1;
        cursor->num_keys = 0;
        
        deleteInternal(parent->keys[leftSibling], parent, cursor);
        free(cursor);
    } 
    // Merge with right sibling
    else if (rightSibling <= parent->num_keys) {
        BPlusTreeNode* rightNode = parent->children[rightSibling];
        
        cursor->keys[cursor->num_keys] = parent->keys[rightSibling - 1];
        
        for (int i = cursor->num_keys + 1, j = 0; j < rightNode->num_keys; j++) {
            cursor->keys[i] = rightNode->keys[j];
        }
        
        for (int i = cursor->num_keys + 1, j = 0; j < rightNode->num_keys + 1; j++) {
            cursor->children[i] = rightNode->children[j];
            rightNode->children[j] = NULL;
        }
        
        cursor->num_keys += rightNode->num_keys + 1;
        rightNode->num_keys = 0;
        
        deleteInternal(parent->keys[rightSibling - 1], parent, rightNode);
        free(rightNode);
    }
}



void printTree(BPlusTreeNode* node, int level) {
    if (node == NULL) {
        return;
    }
    
    printf("Level %d: ", level);
    for (int i = 0; i < node->num_keys; i++) {
        printf("%d ", node->keys[i]);
    }
    printf("\n");
    
    if (!node->is_leaf) {
        for (int i = 0; i <= node->num_keys; i++) {
            printTree(node->children[i], level + 1);
        }
    }
}


// Pharmacy Management Functions
void loadDataFromFile() {
    FILE* file = fopen("pharmacy_data.txt", "r");
    if (file == NULL) {
        printf("No existing data file found. Starting with an empty inventory.\n");
        return;
    }

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

        // Insert into B+ tree using medicationID as key
        insert(newMedication->medicationID, newMedication);

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

    // Insert into B+ tree
    insert(newMedication->medicationID, newMedication);
    printf("Medication added successfully!\n");

    saveDataToFile();
}


void saveDataToFile() {
    FILE* file = fopen("pharmacy_data.txt", "w");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    // Traverse the B+ tree in order (left to right through leaf nodes)
    if (root == NULL) {
        fclose(file);
        return;
    }

    BPlusTreeNode* cursor = root;
    while (!cursor->is_leaf) {
        cursor = cursor->children[0];
    }

    while (cursor != NULL) {
        for (int i = 0; i < cursor->num_keys; i++) {
            Medication* med = cursor->data[i];
            if (med != NULL) {
                // Write medication details to file
                fprintf(file, "%d %s %d %d %.2f %s %d %d\n",
                        med->medicationID, med->medicationName, med->batchNumber,
                        med->quantityInStock, med->pricePerUnit, med->expirationDate,
                        med->reorderLevel, med->totalSales);

                // Write supplier details to file
                Supplier* supplier = med->supplierList;
                while (supplier != NULL) {
                    fprintf(file, "%d %s %d %s\n",
                            supplier->supplierID, supplier->supplierName,
                            supplier->quantitySupplied, supplier->contactInfo);
                    supplier = supplier->next;
                }
                fprintf(file, "END_SUPPLIERS\n");
            }
        }
        cursor = cursor->next;
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

    Medication* med = search(medicationID);
    if (med != NULL && med->batchNumber == batchNumber) {
        printf("Medication found! Enter new details:\n");
        
        printf("Enter new Medication Name: ");
        scanf("%s", med->medicationName);
        printf("Enter new Quantity in Stock: ");
        scanf("%d", &med->quantityInStock);
        printf("Enter new Price per Unit: ");
        scanf("%f", &med->pricePerUnit);
        printf("Enter new Expiration Date (DD-MM-YYYY): ");
        scanf("%s", med->expirationDate);
        printf("Enter new Reorder Level: ");
        scanf("%d", &med->reorderLevel);

        printf("Do you want to update supplier information? (1 for Yes, 0 for No): ");
        int updateSupplier;
        scanf("%d", &updateSupplier);

        if (updateSupplier == 1) {
            Supplier* supplier = med->supplierList;
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
    } else {
        printf("Medication with ID %d and Batch Number %d not found!\n", medicationID, batchNumber);
    }
}


void deleteMedication() {
    int medicationID, batchNumber;
    printf("Enter Medication ID to delete: ");
    scanf("%d", &medicationID);
    printf("Enter Batch Number to delete: ");
    scanf("%d", &batchNumber);

    Medication* med = search(medicationID);
    if (med != NULL && med->batchNumber == batchNumber) {
        // Free supplier data
        Supplier* supplier = med->supplierList;
        while (supplier != NULL) {
            Supplier* temp = supplier;
            supplier = supplier->next;
            free(temp);
        }
        
        // Delete from B+ tree
        deleteKey(medicationID);
        printf("Medication deleted successfully!\n");
        saveDataToFile();
    } else {
        printf("Medication with ID %d and Batch Number %d not found!\n", medicationID, batchNumber);
    }
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

        Medication* med = search(medicationID);
        if (med != NULL) {
            printf("Medication found!\n");
            printf("Medication ID: %d\n", med->medicationID);
            printf("Medication Name: %s\n", med->medicationName);
            printf("Batch Number: %d\n", med->batchNumber);
            printf("Quantity in Stock: %d\n", med->quantityInStock);
            printf("Price per Unit: %.2f\n", med->pricePerUnit);
            printf("Expiration Date: %s\n", med->expirationDate);
            printf("Reorder Level: %d\n", med->reorderLevel);
            printf("Total Sales: %d\n", med->totalSales);

            Supplier* supplier = med->supplierList;
            while (supplier != NULL) {
                printf("Supplier ID: %d\n", supplier->supplierID);
                printf("Supplier Name: %s\n", supplier->supplierName);
                printf("Quantity Supplied: %d\n", supplier->quantitySupplied);
                printf("Contact Info: %s\n", supplier->contactInfo);
                supplier = supplier->next;
            }
        } else {
            printf("Medication with ID %d not found!\n", medicationID);
        }
    } else if (choice == 2) {
        char medicationName[50];
        printf("Enter Medication Name: ");
        scanf("%s", medicationName);

        // Need to traverse the B+ tree to find by name
        int found = 0;
        if (root != NULL) {
            BPlusTreeNode* cursor = root;
            while (!cursor->is_leaf) {
                cursor = cursor->children[0];
            }

            while (cursor != NULL) {
                for (int i = 0; i < cursor->num_keys; i++) {
                    if (strcmp(cursor->data[i]->medicationName, medicationName) == 0) {
                        Medication* med = cursor->data[i];
                        printf("Medication found!\n");
                        printf("Medication ID: %d\n", med->medicationID);
                        printf("Medication Name: %s\n", med->medicationName);
                        printf("Batch Number: %d\n", med->batchNumber);
                        printf("Quantity in Stock: %d\n", med->quantityInStock);
                        printf("Price per Unit: %.2f\n", med->pricePerUnit);
                        printf("Expiration Date: %s\n", med->expirationDate);
                        printf("Reorder Level: %d\n", med->reorderLevel);
                        printf("Total Sales: %d\n", med->totalSales);

                        Supplier* supplier = med->supplierList;
                        while (supplier != NULL) {
                            printf("Supplier ID: %d\n", supplier->supplierID);
                            printf("Supplier Name: %s\n", supplier->supplierName);
                            printf("Quantity Supplied: %d\n", supplier->quantitySupplied);
                            printf("Contact Info: %s\n", supplier->contactInfo);
                            supplier = supplier->next;
                        }
                        found = 1;
                    }
                }
                cursor = cursor->next;
            }
        }

        if (!found) {
            printf("Medication with Name %s not found!\n", medicationName);
        }
    } else if (choice == 3) {
        char supplierName[50];
        printf("Enter Supplier Name: ");
        scanf("%s", supplierName);

        // Need to traverse the B+ tree to find by supplier name
        int found = 0;
        if (root != NULL) {
            BPlusTreeNode* cursor = root;
            while (!cursor->is_leaf) {
                cursor = cursor->children[0];
            }

            while (cursor != NULL) {
                for (int i = 0; i < cursor->num_keys; i++) {
                    Supplier* supplier = cursor->data[i]->supplierList;
                    while (supplier != NULL) {
                        if (strcmp(supplier->supplierName, supplierName) == 0) {
                            Medication* med = cursor->data[i];
                            printf("Medication found!\n");
                            printf("Medication ID: %d\n", med->medicationID);
                            printf("Medication Name: %s\n", med->medicationName);
                            printf("Batch Number: %d\n", med->batchNumber);
                            printf("Quantity in Stock: %d\n", med->quantityInStock);
                            printf("Price per Unit: %.2f\n", med->pricePerUnit);
                            printf("Expiration Date: %s\n", med->expirationDate);
                            printf("Reorder Level: %d\n", med->reorderLevel);
                            printf("Total Sales: %d\n", med->totalSales);

                            printf("Supplier ID: %d\n", supplier->supplierID);
                            printf("Supplier Name: %s\n", supplier->supplierName);
                            printf("Quantity Supplied: %d\n", supplier->quantitySupplied);
                            printf("Contact Info: %s\n", supplier->contactInfo);
                            found = 1;
                        }
                        supplier = supplier->next;
                    }
                }
                cursor = cursor->next;
            }
        }

        if (!found) {
            printf("No medication found for supplier %s!\n", supplierName);
        }
    } else {
        printf("Invalid choice!\n");
    }
}

void generateStockAlerts() {
    if (root == NULL) {
        printf("No medications in inventory!\n");
        return;
    }

    int alertCount = 0;
    BPlusTreeNode* cursor = root;
    while (!cursor->is_leaf) {
        cursor = cursor->children[0];
    }

    printf("\nStock Alerts:\n");
    printf("------------------------------------------------------------\n");

    while (cursor != NULL) {
        for (int i = 0; i < cursor->num_keys; i++) {
            Medication* med = cursor->data[i];
            if (med->quantityInStock < med->reorderLevel) {
                printf("Medication ID: %d\n", med->medicationID);
                printf("Medication Name: %s\n", med->medicationName);
                printf("Batch Number: %d\n", med->batchNumber);
                printf("Quantity in Stock: %d\n", med->quantityInStock);
                printf("Reorder Level: %d\n", med->reorderLevel);
                printf("------------------------------------------------------------\n");
                alertCount++;
            }
        }
        cursor = cursor->next;
    }

    if (alertCount == 0) {
        printf("No stock alerts to display.\n");
    }
}

int isExpiryDateNear(char* expiryDate) {
    // Get the current date
    time_t now = time(NULL);
    struct tm* currentDate = localtime(&now);

    // Parse expiry date (DD-MM-YYYY)
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
    if (root == NULL) {
        printf("No medications in inventory!\n");
        return;
    }

    int alertCount = 0;
    BPlusTreeNode* cursor = root;
    while (!cursor->is_leaf) {
        cursor = cursor->children[0];
    }

    printf("\nExpiration Alerts:\n");
    printf("------------------------------------------------------------\n");

    while (cursor != NULL) {
        for (int i = 0; i < cursor->num_keys; i++) {
            Medication* med = cursor->data[i];
            if (isExpiryDateNear(med->expirationDate)) {
                printf("Medication ID: %d\n", med->medicationID);
                printf("Medication Name: %s\n", med->medicationName);
                printf("Batch Number: %d\n", med->batchNumber);
                printf("Expiration Date: %s\n", med->expirationDate);
                printf("------------------------------------------------------------\n");
                alertCount++;
            }
        }
        cursor = cursor->next;
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


void sortMedicationsByExpiration() {
    if (root == NULL) {
        printf("No medications to sort!\n");
        return;
    }

    // First, collect all medications in an array
    int count = 0;
    Medication* medications[1000]; // Assuming max 1000 medications
    
    BPlusTreeNode* cursor = root;
    while (!cursor->is_leaf) {
        cursor = cursor->children[0];
    }

    while (cursor != NULL) {
        for (int i = 0; i < cursor->num_keys; i++) {
            medications[count++] = cursor->data[i];
        }
        cursor = cursor->next;
    }

    // Simple bubble sort by expiration date
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (compareDates(medications[j]->expirationDate, medications[j+1]->expirationDate) > 0) {
                Medication* temp = medications[j];
                medications[j] = medications[j+1];
                medications[j+1] = temp;
            }
        }
    }

    // Print sorted medications
    printf("\nMedications sorted by expiration date:\n");
    printf("------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("Medication ID: %d\n", medications[i]->medicationID);
        printf("Medication Name: %s\n", medications[i]->medicationName);
        printf("Expiration Date: %s\n", medications[i]->expirationDate);
        printf("------------------------------------------------------------\n");
    }

}



void salesTracking() {
    int medicationID, batchNumber, quantitySold;
    printf("Enter Medication ID: ");
    scanf("%d", &medicationID);
    printf("Enter Batch Number: ");
    scanf("%d", &batchNumber);
    printf("Enter Quantity Sold: ");
    scanf("%d", &quantitySold);

    Medication* med = search(medicationID);
    if (med != NULL && med->batchNumber == batchNumber) {
        if (med->quantityInStock >= quantitySold) {
            med->quantityInStock -= quantitySold;
            med->totalSales += quantitySold;
            printf("Sales recorded successfully!\n");
            saveDataToFile();
        } else {
            printf("Error: Not enough stock available!\n");
        }
    } else {
        printf("Medication with ID %d and Batch Number %d not found!\n", medicationID, batchNumber);
    }
}


void addSupplier() {
    int medicationID, batchNumber;
    printf("Enter Medication ID to add supplier: ");
    scanf("%d", &medicationID);
    printf("Enter Batch Number: ");
    scanf("%d", &batchNumber);

    Medication* med = search(medicationID);
    if (med != NULL && med->batchNumber == batchNumber) {
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
        newSupplier->next = med->supplierList;
        med->supplierList = newSupplier;

        printf("Supplier added successfully!\n");
        saveDataToFile();
    } else {
        printf("Medication with ID %d and Batch Number %d not found!\n", medicationID, batchNumber);
    }
}


void updateSupplier() {
    int supplierID;
    printf("Enter Supplier ID to update: ");
    scanf("%d", &supplierID);

    // Need to traverse the B+ tree to find the supplier
    int found = 0;
    if (root != NULL) {
        BPlusTreeNode* cursor = root;
        while (!cursor->is_leaf) {
            cursor = cursor->children[0];
        }

        while (cursor != NULL) {
            for (int i = 0; i < cursor->num_keys; i++) {
                Supplier* supplier = cursor->data[i]->supplierList;
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
            }
            cursor = cursor->next;
        }
    }

    printf("Supplier with ID %d not found!\n", supplierID);
}


void deleteSupplier() {
    int supplierID;
    printf("Enter Supplier ID to delete: ");
    scanf("%d", &supplierID);

    // Need to traverse the B+ tree to find the supplier
    int found = 0;
    if (root != NULL) {
        BPlusTreeNode* cursor = root;
        while (!cursor->is_leaf) {
            cursor = cursor->children[0];
        }

        while (cursor != NULL) {
            for (int i = 0; i < cursor->num_keys; i++) {
                Supplier* prevSupplier = NULL;
                Supplier* supplier = cursor->data[i]->supplierList;
                
                while (supplier != NULL) {
                    if (supplier->supplierID == supplierID) {
                        if (prevSupplier == NULL) {
                            // If the supplier to delete is the first node
                            cursor->data[i]->supplierList = supplier->next;
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
            }
            cursor = cursor->next;
        }
    }

    printf("Supplier with ID %d not found!\n", supplierID);
}


void searchSupplier() {
    int supplierID;
    printf("Enter Supplier ID to search: ");
    scanf("%d", &supplierID);

    // Need to traverse the B+ tree to find the supplier
    int found = 0;
    if (root != NULL) {
        BPlusTreeNode* cursor = root;
        while (!cursor->is_leaf) {
            cursor = cursor->children[0];
        }

        while (cursor != NULL) {
            for (int i = 0; i < cursor->num_keys; i++) {
                Supplier* supplier = cursor->data[i]->supplierList;
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
            }
            cursor = cursor->next;
        }
    }

    printf("Supplier with ID %d not found!\n", supplierID);
}


void supplierManagement() {
    int ch;
    printf("Enter your choice\n");
    printf("1. Add supplier\n");
    printf("2. Update supplier\n");
    printf("3. Delete supplier\n");
    printf("4. Search supplier\n");
    scanf("%d", &ch);
    switch(ch) {
        case 1: addSupplier(); break;
        case 2: updateSupplier(); break;
        case 3: deleteSupplier(); break;
        case 4: searchSupplier(); break;
        default: printf("Invalid choice!\n");
    }
}


void findAllRounderSuppliers() {
    if (root == NULL) {
        printf("No medications in inventory!\n");
        return;
    }

    int maxUniqueMedications = 0;
    Supplier* allRounderSupplier = NULL;

    // First, collect all suppliers and count unique medications they supply
    Supplier* suppliers[1000]; // Assuming max 1000 suppliers
    int supplierCount = 0;
    int uniqueCounts[1000] = {0};

    BPlusTreeNode* cursor = root;
    while (!cursor->is_leaf) {
        cursor = cursor->children[0];
    }

    while (cursor != NULL) {
        for (int i = 0; i < cursor->num_keys; i++) {
            Supplier* supplier = cursor->data[i]->supplierList;
            while (supplier != NULL) {
                // Check if we've seen this supplier before
                int found = 0;
                for (int j = 0; j < supplierCount; j++) {
                    if (suppliers[j]->supplierID == supplier->supplierID) {
                        found = 1;
                        break;
                    }
                }
                
                if (!found) {
                    suppliers[supplierCount] = supplier;
                    supplierCount++;
                }
                
                supplier = supplier->next;
            }
        }
        cursor = cursor->next;
    }

    // Now count unique medications for each supplier
    for (int i = 0; i < supplierCount; i++) {
        cursor = root;
        while (!cursor->is_leaf) {
            cursor = cursor->children[0];
        }

        while (cursor != NULL) {
            for (int j = 0; j < cursor->num_keys; j++) {
                Supplier* supplier = cursor->data[j]->supplierList;
                while (supplier != NULL) {
                    if (supplier->supplierID == suppliers[i]->supplierID) {
                        uniqueCounts[i]++;
                        break; // Count each medication only once per supplier
                    }
                    supplier = supplier->next;
                }
            }
            cursor = cursor->next;
        }

        if (uniqueCounts[i] > maxUniqueMedications) {
            maxUniqueMedications = uniqueCounts[i];
            allRounderSupplier = suppliers[i];
        }
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
    if (root == NULL) {
        printf("No medications in inventory!\n");
        return;
    }

    float maxTurnover = 0;
    Supplier* largestTurnoverSupplier = NULL;

    // First, collect all suppliers
    Supplier* suppliers[1000]; // Assuming max 1000 suppliers
    int supplierCount = 0;
    float turnovers[1000] = {0};

    BPlusTreeNode* cursor = root;
    while (!cursor->is_leaf) {
        cursor = cursor->children[0];
    }

    while (cursor != NULL) {
        for (int i = 0; i < cursor->num_keys; i++) {
            Supplier* supplier = cursor->data[i]->supplierList;
            while (supplier != NULL) {
                // Check if we've seen this supplier before
                int found = 0;
                for (int j = 0; j < supplierCount; j++) {
                    if (suppliers[j]->supplierID == supplier->supplierID) {
                        found = 1;
                        break;
                    }
                }
                
                if (!found) {
                    suppliers[supplierCount] = supplier;
                    supplierCount++;
                }
                
                supplier = supplier->next;
            }
        }
        cursor = cursor->next;
    }

    // Now calculate turnover for each supplier
    for (int i = 0; i < supplierCount; i++) {
        cursor = root;
        while (!cursor->is_leaf) {
            cursor = cursor->children[0];
        }

        while (cursor != NULL) {
            for (int j = 0; j < cursor->num_keys; j++) {
                Supplier* supplier = cursor->data[j]->supplierList;
                while (supplier != NULL) {
                    if (supplier->supplierID == suppliers[i]->supplierID) {
                        turnovers[i] += supplier->quantitySupplied * cursor->data[j]->pricePerUnit;
                        break; // Each medication only once
                    }
                    supplier = supplier->next;
                }
            }
            cursor = cursor->next;
        }

        if (turnovers[i] > maxTurnover) {
            maxTurnover = turnovers[i];
            largestTurnoverSupplier = suppliers[i];
        }
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

    while (1) {
        int medicationID, batchNumber, quantityInStock, reorderLevel, totalSales;
        char medicationName[50], expirationDate[11];
        float pricePerUnit;

        // Read medication details
        if (fscanf(file, "%d %s %d %d %f %s %d %d",
                   &medicationID, medicationName, &batchNumber, &quantityInStock,
                   &pricePerUnit, expirationDate, &reorderLevel, &totalSales) != 8) {
            break; // Stop reading medications
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

        // Read and print all supplier details
        while (1) {
            int supplierID, quantitySupplied;
            char supplierName[50], contactInfo[50], buffer[20];
            
            // Peek at the next token to check for END_SUPPLIERS
            long pos = ftell(file); // Save current position
            if (fscanf(file, "%s", buffer) != 1) break;
            
            // If it's END_SUPPLIERS, we're done with suppliers
            if (strcmp(buffer, "END_SUPPLIERS") == 0) {
                break;
            }
            
            // Otherwise, rewind and read supplier data
            fseek(file, pos, SEEK_SET); // Go back to previous position
            
            if (fscanf(file, "%d %s %d %s", &supplierID, supplierName, 
                      &quantitySupplied, contactInfo) != 4) {
                break; // Stop if we can't read a complete supplier record
            }

            // Print supplier details
            printf("  Supplier ID: %d\n", supplierID);
            printf("  Supplier Name: %s\n", supplierName);
            printf("  Quantity Supplied: %d\n", quantitySupplied);
            printf("  Contact Info: %s\n", contactInfo);
        }

        printf("------------------------------------------------------------\n");
    }

    fclose(file);
}