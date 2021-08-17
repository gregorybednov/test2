#include "application.h"

struct department {
	wchar_t* name;
	struct listNode* employment;
};

struct employment {
	wchar_t* name;
	wchar_t* surname;
	wchar_t* middleName;
	wchar_t* function;
	uint_least32_t salary;
};

enum optype {
    NEXT_EMPLOYMENT, PREV_EMPLOYMENT,
    NEXT_DEPARTMENT, PREV_DEPARTMENT,
    EDIT_ARGUMENT,
    ADD_EMPLOYMENT, DEL_EMPLOYMENT,
    ADD_DEPARTMENT, DEL_DEPARTMENT
};

struct operation {
    enum optype otype;
    enum argtype atype;
    union argval arg;
};

struct listNode* departmentList = NULL;
struct semistack operations;

void freeOperation (void* pOperation) {
    if (pOperation==NULL) return;
    switch (((struct operation*)pOperation)->atype) {
        case NONE:
        case EMPLOYMENT_SALARY:
                break;
        case EMPLOYMENT_SURNAME:
        case EMPLOYMENT_NAME:
        case EMPLOYMENT_MIDDLENAME:
        case EMPLOYMENT_FUNCTION:
        case DEPARTMENT_NAME:
            free(((struct operation*)pOperation)->arg.asString);
            break;
        case POINTER:
            free(((struct operation*)pOperation)->arg.asPointer);
            break;
    }
    free(pOperation);
}

struct operation* newOperation(enum optype optype, enum  argtype atype, union argval x){
    struct operation *result = malloc(sizeof(struct operation));
    if (result==NULL) return NULL;
    result->arg = x;
    result->atype = atype;
    result->otype = optype;
    return result;
}

int newDepartmentOperation() {
    struct department* newDepartment = malloc(sizeof(struct department));
    if (newDepartment==NULL) return 0;

    struct operation* op; union argval x = {.asPointer=newDepartment};
    if ((op = newOperation(ADD_DEPARTMENT, POINTER, x ))==NULL) {free(newDepartment); return 0;}
    pushSemistack(&operations, op, freeOperation);
    newDepartment->employment = NULL;
    newAfter(&departmentList, newDepartment);
    return 1;
}

int newEmploymentOperation() {
    if (departmentList==NULL) return 0 ;
    if (departmentList->item==NULL) return 0;
    struct employment* newEmploynemt = malloc(sizeof(struct employment));
    if (newEmploynemt==NULL) return 0;
    struct operation* op; union argval x = {.asPointer=newEmploynemt};
    if ((op = newOperation(ADD_EMPLOYMENT, POINTER, x ))==NULL) {free(newEmploynemt); return 0;}
    newEmploynemt->surname = NULL;
    newEmploynemt->name = NULL;
    newEmploynemt->middleName = NULL;
    newEmploynemt->function = NULL;
    newEmploynemt->salary = 0;
    newAfter(&(((struct department*)(departmentList->item))->employment), newEmploynemt);
    pushSemistack(&operations, op, freeOperation);
    return 1;
}

struct department* deleteDepartment() {
    if (departmentList==NULL) return NULL;
    struct department* currentDepartment = departmentList->item;
    deleteNode(&departmentList);
    return currentDepartment;
}

void deleteDepartmentOperation() {
    struct department *wasDeleted = deleteDepartment();
    if (wasDeleted==NULL) return;
    struct operation* op; union argval x = {.asPointer = wasDeleted};
    if ((op = newOperation(DEL_DEPARTMENT, POINTER, x ))==NULL) return;
    pushSemistack(&operations, op, freeOperation);
}

struct employment* deleteEmployment() { /* returns 'deleted' (from the structures) employment */
    if (departmentList==NULL) return NULL;
    if (departmentList->item == NULL) return NULL;
    struct employment* currentEmployment = ((struct listNode*)(((struct department*)departmentList->item)->employment))->item;
    deleteNode(&(((struct department *) departmentList->item)->employment));
    return currentEmployment;
}

void deleteEmploymentOperation() {
    struct employment* wasDeleted = deleteEmployment();
    if (wasDeleted==NULL) return;
    struct operation* op; union argval x = {.asPointer = wasDeleted};
    if ((op = newOperation(DEL_EMPLOYMENT, POINTER, x ))==NULL) return;
    pushSemistack(&operations, op, freeOperation);
}

void setAttributeOperation (enum argtype member, union argval x) {
    struct employment* currentEmployment = NULL;
    if (departmentList==NULL) return;
    if (member==NONE || member==POINTER) return;

    int employmentExists = (((struct department*)(departmentList->item))->employment != NULL);
    if (employmentExists) currentEmployment = (struct employment*)(((struct listNode*)(((struct department*)(departmentList->item))->employment))->item);

    struct operation *op = malloc(sizeof(struct operation));
    if (op==NULL) return;
    op->otype = EDIT_ARGUMENT;
    op->atype = member;
    switch (member) {
        case EMPLOYMENT_SURNAME:
            op->arg.asString = currentEmployment->surname;
            currentEmployment->surname = x.asString;
            break;
        case EMPLOYMENT_NAME:
            op->arg.asString = currentEmployment->name;
            currentEmployment->name = x.asString;
            break;
        case EMPLOYMENT_MIDDLENAME:
            op->arg.asString = currentEmployment->middleName;
            currentEmployment->middleName = x.asString;
            break;
        case EMPLOYMENT_FUNCTION:
            op->arg.asString = currentEmployment->function;
            currentEmployment->function = x.asString;
            break;
        case EMPLOYMENT_SALARY:
            op->arg.asInt = currentEmployment->salary;
            currentEmployment->salary = x.asInt;
            break;
        case DEPARTMENT_NAME:
            op->arg.asString = ((struct department*)departmentList->item)->name;
            ((struct department*)departmentList->item)->name = x.asString;
            break;
    }
    pushSemistack(&operations, op, freeOperation);
}

void printAttribute (FILE *openedFout, enum argtype member) {
    if (departmentList==NULL) return;
    if (departmentList->item == NULL || member==NONE) return;
    if (member==DEPARTMENT_NAME) {
        fwprintf(openedFout, ((struct department*)departmentList->item)->name);
        return;
    }
    struct employment* currentEmployment = (struct employment*)((struct listNode*)(((struct department*)(((struct listNode*)departmentList)->item))->employment)->item);
    switch (member) {
        case NONE:
            break;
        case EMPLOYMENT_SURNAME:
            fwprintf(openedFout, L"%ls", currentEmployment->surname);
            break;
        case EMPLOYMENT_NAME:
            fwprintf(openedFout, L"%ls", currentEmployment->name);
            break;
        case EMPLOYMENT_MIDDLENAME:
            fwprintf(openedFout, L"%ls", currentEmployment->middleName);
            break;
        case EMPLOYMENT_FUNCTION:
            fwprintf(openedFout, L"%ls", currentEmployment->function);
            break;
        case EMPLOYMENT_SALARY:
            fwprintf(openedFout, L"%" PRIuLEAST32, currentEmployment->salary);
            break;
        default:
            break;
    }
}

uint_least32_t getSalary() {
	if (departmentList==NULL) return 0;
	struct listNode* employmentList = ((struct department*)(departmentList->item))->employment;
	if (employmentList==NULL) return 0;
	return ((struct employment*)employmentList->item)->salary;
}

int nextDepartment() {
    if (departmentList==NULL) return 0;
    departmentList = departmentList->next;
    return 1;
}

int nextDepartmentOperation() {
	struct operation *op; union argval x = {.asInt=0};
	if ((op = newOperation(NEXT_DEPARTMENT, NONE, x ))==NULL) return 0;
	int result = nextDepartment();
	if (result) pushSemistack(&operations, op, freeOperation); else free(op);
	return result;
}

int nextEmployment(){
    if (departmentList==NULL) return 0;
    if (((struct department*)(departmentList->item))->employment==NULL) return 0;
    ((struct department*)(departmentList->item))->employment = ((struct department*)(departmentList->item))->employment->next;
    return 1;
}

int nextEmploymentOperation() {
	struct operation *op; union argval x = {.asInt=0};
	if ((op = newOperation(NEXT_EMPLOYMENT, NONE, x ))==NULL) return 0;
	int result = nextEmployment();
	if (result) pushSemistack(&operations, op, freeOperation); else free(op);
	return result;
}

int previousDepartment() {
    if (departmentList==NULL) return 0;
    struct listNode *start = departmentList;
    while (departmentList->next != start) {
        departmentList = departmentList->next;
    }
    return 1;
}

int previousDepartmentOperation() {
    struct operation *op; union argval x = {.asInt=0};
    if ((op = newOperation(PREV_DEPARTMENT, NONE, x ))==NULL) return 0;
    int result = previousDepartment();
    if (result) pushSemistack(&operations, op, freeOperation); else free(op);
    return result;
}

int previousEmployment() {
    if (departmentList==NULL) return 0;
    struct listNode *start = (struct listNode*)departmentList->item;
    while ((struct listNode*)((struct listNode*)departmentList->item)->next != start) {
        departmentList->item = (void*)((struct listNode*)((struct listNode*)departmentList->item))->next;
    }
    return 1;
}

int previousEmploymentOperation() {
    struct operation *op; union argval x = {.asInt=0};
    if ((op = newOperation(PREV_EMPLOYMENT, NONE, x ))==NULL) return 0;
    int result = previousEmployment();
    if (result) pushSemistack(&operations, op, freeOperation); else free(op);
    return result;
}

void* counterFunction(void *content, void* otherArgs) { (*((size_t*)otherArgs))++; return NULL; }

size_t employmentsInDepartment() {
    size_t result = 0;
    if (departmentList==NULL) return 0;
    foreach(((struct department*)departmentList->item)->employment, counterFunction, (void*)(&result));
    return result;
}

size_t departmentsInCompany(){
    size_t result = 0;
    if (departmentList==NULL) return 0;
    foreach(departmentList, counterFunction, (void*)(&result));
    return result;
}

int swapArguments (struct operation *op) {
    wchar_t *tmpWS;
    uint_least32_t tmpInt;
    if (op->atype == NONE || op->atype == POINTER) return 0;
    if (op->atype == DEPARTMENT_NAME) {
        tmpWS = ((struct department*)(departmentList->item))->name;
        ((struct department*)(departmentList->item))->name = op->arg.asString;
        op->arg.asString = tmpWS;
    }

    if (departmentList->item==NULL) return 0;
    struct employment* currentEmployment = ((struct employment*)(((struct listNode*)(((struct department*)(departmentList->item))->employment))->item));
    switch (op->atype){
        case EMPLOYMENT_SURNAME:
            tmpWS = currentEmployment->surname;
            currentEmployment->surname = op->arg.asString;
            op->arg.asString = tmpWS;
            break;
            case EMPLOYMENT_NAME:
                tmpWS = currentEmployment->name;
                currentEmployment->name = op->arg.asString;
                op->arg.asString = tmpWS;
                break;
            case EMPLOYMENT_MIDDLENAME:
                tmpWS = currentEmployment->middleName;
                currentEmployment->middleName = op->arg.asString;
                op->arg.asString = tmpWS;
                break;
            case EMPLOYMENT_FUNCTION:
                tmpWS = currentEmployment->function;
                currentEmployment->function = op->arg.asString;
                op->arg.asString = tmpWS;
                break;
            case EMPLOYMENT_SALARY:
                tmpInt = currentEmployment->salary;
                currentEmployment->salary = op->arg.asInt;
                op->arg.asInt = tmpInt;
                break;
    }
    return 1;
}

int cancel() {
    struct operation* cancelled = (struct operation*)notPopSemistack(&operations);
    if (cancelled==NULL) return 0;
    switch (cancelled->otype) {
        case NEXT_EMPLOYMENT:
            return previousEmployment();
        case PREV_EMPLOYMENT:
            return nextEmployment();
        case NEXT_DEPARTMENT:
            return previousDepartment();
        case PREV_DEPARTMENT:
            return nextDepartment();
        case EDIT_ARGUMENT:
            return swapArguments(cancelled);
        case ADD_EMPLOYMENT:
            if (deleteEmployment()!=NULL) return previousEmployment();
            return 0;
        case DEL_EMPLOYMENT:
            if (departmentList==NULL) return 0;
            return newAfter((struct listNode **) &(departmentList->item), cancelled->arg.asPointer);
        case ADD_DEPARTMENT:
            if (deleteDepartment()) return previousDepartment();
            return 0;
        case DEL_DEPARTMENT:
            return newAfter(&departmentList, cancelled->arg.asPointer);
        default:
            return 0;
    }
    return 1;
}

int retry() {
    struct operation* retried = (struct operation*)notPushSemistack(&operations);
    if (retried==NULL) return 0;
    switch (retried->otype){
        case NEXT_EMPLOYMENT:
            return nextEmployment();
        case PREV_EMPLOYMENT:
            return previousEmployment();
        case NEXT_DEPARTMENT:
            return nextDepartment();
        case PREV_DEPARTMENT:
            return previousDepartment();
        case EDIT_ARGUMENT:
            return swapArguments(retried);
        case ADD_EMPLOYMENT:
            if (departmentList==NULL) return 0;
            return newAfter((struct listNode **) &(departmentList->item), retried->arg.asPointer);

        case DEL_EMPLOYMENT:
            return deleteEmployment()!=NULL;
        case ADD_DEPARTMENT:
            return newAfter(&departmentList, retried->arg.asPointer);
        case DEL_DEPARTMENT:
            return deleteDepartment()!=NULL;
    }
}

void *freeEmployment(void *item, void* otherArgs){
    if (item==NULL) return NULL;
    struct employment* currEmployment = (struct employment*)item;
    free(currEmployment->surname);
    free(currEmployment->name);
    free(currEmployment->middleName);
    free(currEmployment->function);
    free(currEmployment);
    return NULL;
}

void *freeDepartment(void *item, void* otherArgs){
    if (item==NULL) return NULL;
    struct department * currDepartment = (struct department*)item;
    foreach(currDepartment->employment, freeEmployment, NULL);
    free(currDepartment->employment);
    free(currDepartment->name);
    free(currDepartment);
    return NULL;
}

void finalization() {
    foreach(departmentList, freeDepartment, NULL);
    while (deleteNode(&departmentList)) ;
    freeSemistack(&operations, freeOperation);
}

void freeOperationOnly(void *item) {
    free(item);
}

void forgetOperationsHistory(FILE *fin) {
    freeSemistack(&operations, freeOperationOnly);
}