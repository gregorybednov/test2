#include "application.h"
#include <inttypes.h>
#include <locale.h>
#include <wctype.h>
#include <stdlib.h>
#include <stdio.h>

#pragma region error
int checkArgumentsFails(int argc, char **argv, FILE **fin){
    if (argc!=2) return 1;
    *fin = fopen(argv[1], "r");
    if (*fin==NULL) return 2;
    return 0;
}
int argumentsError(int argErrCode) {
    switch (argErrCode) {
        case 1:
            fwprintf(stderr, L"Ошибка: аргумент должен быть точно один (имя XML-файла), не больше и не меньше");
            break;
        case 2:
            fwprintf(stderr, L"Ошибка: файл не был открыт. Проверьте:\n1) правильность написания имени файла;\n2) права доступа на чтение (попробуйте открыть файл)");
            break;
        default:
            break;
    }
    return argErrCode;
}
#pragma endregion

int getXMLprolog (FILE *fin) {
    wchar_t prolog[] = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    size_t index = 0;
    wint_t c = fgetwc(fin);
    if (c!=L'<') return 3;
    index = 1;
    c = fgetwc(fin);
    if (c!=L'?') {
        ungetwc(c, fin); ungetwc(L'<',fin);
        return 0;
    }
    while (prolog[index]==c && !feof(fin) && index<sizeof(prolog)*sizeof(wchar_t)) {
        c = fgetwc(fin);
        index++;
    }
    if (index!=sizeof(prolog)*sizeof(wchar_t)) return 4;
    return 0;
}

void listener (enum argtype atype, FILE* fin) {
    wint_t c;
    size_t index = 0;
    const size_t minsize =  10*sizeof(wchar_t);
    size_t currsize = minsize;
    wchar_t *result = malloc(minsize);
    if (result==NULL) return;
    while (!feof(fin) && (c=fgetwc(fin))!=L'<' ) {
        if (index*sizeof(wchar_t)==currsize) result = realloc(result, currsize*=2);
        result[index++] = (wchar_t)c;
    }
    if (index==0) { free(result); return; }
    if (index*sizeof(wchar_t)==currsize) result = realloc(result, currsize+sizeof(wchar_t));
    result[index] = L'\0';
    union argval x;
    if (atype==EMPLOYMENT_SALARY) x.asInt = (uint_least32_t)wcstol(result, NULL, 10);
    else x.asString = result;
    setAttributeOperation(atype, x);
    if (atype==EMPLOYMENT_SALARY) free(result);
}

void gotoNextTag(FILE *fin) {
    wint_t c;
    while ((c=fgetwc(fin))!=L'<' && !feof(fin)) ;
}

int reallocFlag (void **ptrmem, size_t newsize) {
    void *tmp = realloc(*ptrmem, newsize);
    if (tmp==NULL) return 0;
    *ptrmem = tmp;
    return 1;
}

wchar_t *fgetwsUntilNotC (FILE *fin, int (stop)(wint_t), wint_t *lastC) {
    const size_t mincpty = 10*sizeof(wchar_t);
    size_t currcpty = mincpty;
    wchar_t *result = malloc(mincpty);
    if (result==NULL) return NULL;
    size_t index = 0;
    wint_t c = fgetwc(fin);
    while (!stop((wchar_t)c) && !feof(fin)) {
        if (index*sizeof(wchar_t)==currcpty) result = realloc(result, currcpty*=2);
        result[index++] = (wchar_t)c;
        c = fgetwc(fin);
    }
    *lastC = c;
    if (index==0) {
        free(result); return NULL;
    }
    if (index*sizeof(wchar_t)==currcpty)  result = realloc(result, sizeof(wchar_t)+currcpty);
    result[index] = L'\0';
    return result;
}

int stopAttribute (wint_t c){return (c==L'>')|| c==L'=' || iswblank(c);}

int stopTag(wint_t c){ return iswblank(c) || c==L'>';}

wchar_t *readTagName (FILE *fin, wint_t *reachedEnd) {
    wint_t lastC;
    wchar_t *result = fgetwsUntilNotC(fin, stopTag, &lastC);
    *reachedEnd = (lastC=='>');
    return result;
}

wchar_t *fgetQuotedWS (FILE *fin) {
    const size_t mincpty = 10*sizeof(wchar_t);
    size_t currcpty = mincpty;
    size_t index = 0;
    wint_t quoteSign = fgetwc(fin);
    if (quoteSign!=L'\'' && quoteSign!=L'\"') return NULL;
    wchar_t *result = malloc(mincpty);
    wint_t c = fgetwc(fin);
    int escape = c==L'\\';
    while (!feof(fin) && (c!=quoteSign || escape)){
        if (escape) {
            escape = 0;
        } else {
            escape = c==L'\\';
            if (index*sizeof(wchar_t)==currcpty) result = realloc(result, currcpty*=2);
            result[index++] = (wchar_t)c;
        }
        c = fgetwc(fin);
    }
    if (index==0) {
        free(result);
        return NULL;
    }
    if (index*sizeof(wchar_t)==currcpty) result = realloc(result, sizeof(wchar_t)+currcpty);
    result[index] = L'\0';
    return result;
}

struct tagAttribute {
    wchar_t *memberName;
    wchar_t *memberValue;
};

struct tagAttribute* getAttributes (FILE *fin) {
    const int mincpty = sizeof(struct tagAttribute)*10;
    size_t currcpty = mincpty;
    size_t index = 0;
    struct tagAttribute *result = NULL;
    result = malloc(currcpty);
    if (result==NULL) return NULL;
    result[0].memberName = NULL;
    result[0].memberValue = NULL;
    wint_t lastC = L' ';
    while (lastC!=L'>' && !feof(fin)) {
        if (index*sizeof(struct tagAttribute)==currcpty) result = realloc(result, currcpty*=2);
        wint_t c = L' ';
        while (!feof(fin) && c==L' ') {
            c = fgetwc(fin);
        }
        if (c!=L' ') ungetwc(c, fin);
        result[index].memberName = fgetwsUntilNotC(fin, stopAttribute, &lastC);
        if (lastC==L'=') result[index].memberValue = fgetQuotedWS(fin);
        else result[index].memberValue = NULL;
        index++;
    }
    if (index==0) {
        free(result);
        return NULL;
    }
    if (result[index-1].memberName==NULL && result[index-1].memberValue==NULL) return result;
    if (index*sizeof(struct tagAttribute)==currcpty) result = realloc(result, sizeof(struct tagAttribute)+currcpty);
    result[index].memberValue = NULL;
    result[index].memberName  = NULL;
    return result;
}

void tagActivation(wchar_t *tagName, struct tagAttribute *tagAttributes, FILE *fin){
    const wchar_t DEP_TAG[] = L"department";
    const wchar_t DEP_ATT[] = L"name";
    const wchar_t EMP_TAG[] = L"employment";
    const wchar_t ESN_TAG[] = L"surname";
    const wchar_t ENM_TAG[] = L"name";
    const wchar_t EMN_TAG[] = L"middleName";
    const wchar_t EFN_TAG[] = L"function";
    const wchar_t ESL_TAG[] = L"salary";
    if (tagName==NULL) return;
    if (wcscmp(DEP_TAG, tagName)==0) {
        newDepartmentOperation();
        nextDepartmentOperation();
    }
    if (wcscmp(EMP_TAG, tagName)==0) {
        newEmploymentOperation();
        nextEmploymentOperation();
    }
    if (wcscmp(ESN_TAG, tagName)==0) listener(EMPLOYMENT_SURNAME, fin);
    if (wcscmp(ENM_TAG, tagName)==0) listener(EMPLOYMENT_NAME, fin);
    if (wcscmp(EMN_TAG, tagName)==0) listener(EMPLOYMENT_MIDDLENAME, fin);
    if (wcscmp(EFN_TAG, tagName)==0) listener(EMPLOYMENT_FUNCTION, fin);
    if (wcscmp(ESL_TAG, tagName)==0) listener(EMPLOYMENT_SALARY, fin);
    if (tagAttributes==NULL) return;
    size_t index = 0;
    while (tagAttributes[index].memberValue!=NULL || tagAttributes[index].memberName!=NULL){
        union argval tmp;
        tmp.asString = tagAttributes->memberValue;
        if (tagAttributes[index].memberName!=NULL) {
            if (wcscmp(DEP_ATT, tagAttributes[index].memberName)==0) {
                setAttributeOperation(DEPARTMENT_NAME, tmp);
                tagAttributes[index].memberValue = NULL;
            }
        }
        index++;
    }
}

void tagProcessing(FILE *fin, int *noerror){
    wchar_t *tagName = NULL;
    struct tagAttribute *tagAttributes = NULL;

    gotoNextTag(fin);
    wint_t noAttributes = 0;
    if ((tagName=readTagName(fin,&noAttributes))==NULL) return;
    if (!noAttributes) tagAttributes = getAttributes(fin);
    tagActivation(tagName, tagAttributes, fin);
    free(tagName);
    if (tagAttributes==NULL) return;
    size_t index = 0;
    while (tagAttributes[index].memberName!=NULL ||  tagAttributes[index].memberValue!=NULL) {
        free(tagAttributes[index].memberValue);
        free(tagAttributes[index].memberName);
        ++index;
    }
    free(tagAttributes);
}

#pragma region printFunctions
void printTree() {
    size_t dInC = departmentsInCompany();
    nextDepartmentOperation();
    for (size_t i = 0; i < dInC; ++i) {
        printAttribute(stdout, DEPARTMENT_NAME);
        size_t eInD = employmentsInDepartment();
        nextEmploymentOperation();
        for (size_t j = 0; j < eInD; ++j) {
            wprintf(L"\n%5c",' ');printAttribute(stdout, EMPLOYMENT_SURNAME);
            wprintf(L"\n%5c",' ');printAttribute(stdout, EMPLOYMENT_NAME);
            wprintf(L"\n%5c",' ');printAttribute(stdout, EMPLOYMENT_MIDDLENAME);
            wprintf(L"\n%5c",' ');printAttribute(stdout, EMPLOYMENT_FUNCTION);
            wprintf(L"\n%5c",' ');printAttribute(stdout, EMPLOYMENT_SALARY);
            wprintf(L"\n");
            nextEmploymentOperation();
        }
        nextDepartmentOperation();
    }
}

void printAverageSalary() {
    size_t eInD = employmentsInDepartment();
    uint_least32_t sum = 0;
    for (size_t i = 0; i<eInD; i++) {
        sum+=getSalary();
        nextEmploymentOperation();
    }
    fwprintf(stdout, L"Средняя зарплата по отделу составляет %" PRIdLEAST32 " (%lf)", sum/eInD, ((double)sum)/eInD );
}

void printMenu() {
    fwprintf(stdout, L"\n\nВыберите одно из действий:\n");
    fwprintf(stdout, L"0. Завершить работу\n1. Отменить предыдущее действие\n2. Вернуть отмену\n3. Перейти к следующему отделу (циклично)\n4. Перейти к следующему работнику отдела (циклично)");
    fwprintf(stdout, L"\n5. Перейти к предыдущему работнику отдела (если список длинный, операции 5 и 6 могут быть долгими - зависит от длины списка сотрудников линейно)\n6.Перейти к предыдущему отделу");
    fwprintf(stdout, L"\n7. Удалить следующий отдел (если в списке отделов отдел один, то удаляется именно он, так как он же следующий)");
    fwprintf(stdout, L"\n8. Удалить следующего сотрудника (аналогично п.7)\n(доп.опция) 9. Вывести среднюю з/п текущего отдела (циклически пролистывает весь список сотрудников отдела, извлекая зарплаты - влияет на историю операций).");
    fwprintf(stdout, L"\n10. Прочитать/отредактировать поле сотрудника/отдела.\nВыберите нужный пункт: ");
}
#pragma endregion

int getAction (void) {
    int result;
    const int N = 11;
    do {
        wscanf(L"%d", &result);
    } while (result >= N); // N число доступных операций
    return result;
}

void editingDialog() {
    int menuOption, optionAttribute;
    wprintf(L"Выберите\n1, если хотите посмотреть какие-то данные\n");
    wprintf(L"2, если хотите редактировать какие-то данные\n");
    wprintf(L"любое другое число, чтобы выйти в главное меню\n");
    wprintf(L"Вторым числом выберите, какой именно атрибут хотите отредактировать:\n");
    wprintf(L"\t1) название отдела;\n\t2) фамилия сотрудника;\n\t3) имя сотрудника;\n\t4) отчество сотрудника\n");
    wprintf(L"\t5) функцию сотрудника;\n\t6) зарплату сотрудника\n");
    wprintf(L"Выбор: "); wscanf(L"%d", &menuOption);
    if (menuOption != 1 && menuOption !=2) return;
    wprintf(L"Выберите номер поля:");
    wscanf(L"%d", &optionAttribute);
    wprintf(L"Введите в кавычках значение поля:\n\n");

    wchar_t *value = NULL;
    {
        wint_t c;
        while (iswblank(c = getwc(stdin))) ;
        ungetwc(c,stdin);
    }
    if (menuOption==2) value = fgetQuotedWS(stdin);
    union argval x = {.asString=value};
    switch (optionAttribute) {
        case 1:
            if (menuOption==1) printAttribute(stdout, DEPARTMENT_NAME);
            else setAttributeOperation(DEPARTMENT_NAME, x);
            break;
        case 2:
            if (menuOption==1) printAttribute(stdout, EMPLOYMENT_SURNAME);
            else setAttributeOperation(EMPLOYMENT_SURNAME, x);
            break;
        case 3:
            if (menuOption==1) printAttribute(stdout, EMPLOYMENT_NAME);
            else setAttributeOperation(EMPLOYMENT_NAME, x);
            break;
        case 4:
            if (menuOption==1) printAttribute(stdout, EMPLOYMENT_MIDDLENAME);
            else setAttributeOperation(EMPLOYMENT_MIDDLENAME, x);
            break;
        case 5:
            if (menuOption==1) printAttribute(stdout, EMPLOYMENT_FUNCTION);
            else setAttributeOperation(EMPLOYMENT_FUNCTION, x);
            break;
        case 6:
            if (menuOption==1) printAttribute(stdout, EMPLOYMENT_SALARY);
            else {
                x.asInt = wcstol(value, NULL, 10);
                setAttributeOperation(EMPLOYMENT_SALARY, x);
                free(value);
            }
            break;
        default:
            free(value);
            break;
    }
}

void doAction (int menuAction) {
    switch (menuAction) {
        case 0:
            wprintf(L"Завершение работы...");
            break;
        case 1:
            cancel();
            break;
        case 2:
            retry();
            break;
        case 3:
            nextDepartmentOperation();
            break;
        case 4:
            nextEmploymentOperation();
            break;
        case 5:
            previousDepartmentOperation();
        case 6:
            previousEmploymentOperation();
        case 7:
            deleteDepartmentOperation();
            break;
        case 8:
            deleteEmploymentOperation();
            break;
        case 9:
            printAverageSalary();
            break;
        case 10:
            editingDialog();
            break;
    }
}

int main(int argc, char **argv) {
    FILE *fin;
    setlocale(LC_ALL, "");
    if (checkArgumentsFails(argc, argv, &fin)) return argumentsError(checkArgumentsFails(argc, argv, &fin));
    getXMLprolog(fin);
    int noerror = 1;
    while (!feof(fin) && noerror) {
        tagProcessing(fin, &noerror);
    }
    fclose(fin);
    printTree();
    forgetOperationsHistory();
    int menuAction = 1;
    fwprintf(stdout, L"\nВ данный момент Вы обозреваете первый отдел (если такой есть) и первого сотрудника.\n");
    while (menuAction) {
        printMenu();
        menuAction = getAction();
        doAction(menuAction);
    }
    finalization();
    return 0;
}
