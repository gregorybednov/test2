#include <wchar.h>
#include <stdio.h>
#include <inttypes.h>
#include "list.h"
#include "semistack.h"

union argval {
	wchar_t *asString;
	uint_least32_t asInt;
	void* asPointer;
};

enum argtype {
    NONE,
	EMPLOYMENT_SURNAME,
	EMPLOYMENT_NAME,
	EMPLOYMENT_MIDDLENAME,
	EMPLOYMENT_FUNCTION,
	EMPLOYMENT_SALARY,
	DEPARTMENT_NAME,
    POINTER
};

/* Operation: creates new department
 (if were cancelled and retried, just 'wakes up' created).
 Returns 1 if success and 0 if failed. */
int newDepartmentOperation();

/* Operation: creates new employment in CURRENT and EXISTING department.
 (if were cancelled and retried, just 'wakes up' created).
 Returns 1 if success and 0 if failed. */
int newEmploymentOperation();

/* Operation: deletes CURRENT and EXISTING department.
Returns 1 if success and 0 if failed. */
void deleteDepartmentOperation();

/* Operation: deletes CURRENT and EXISTING employer in C-t&E-ing department.
Returns 1 if success and 0 if failed. */
void deleteEmploymentOperation();

/* Operation: sets attribute typed member of department or employment.*/
void setAttributeOperation (enum argtype member, union argval x);

/* Prints the value of 'member' attribute of employment or department in OPENED fout.
Is NOT an operation of data structure, so cannot be cancelled or retried. */
void printAttribute (FILE *fout, enum argtype member);

/* Returns the 'salary' value of current (and of course existing) employment.
 If it's unavailable, returns 0 anyway. */
uint_least32_t getSalary();

/* Operation: goes to next employment in list. Returns 0 if there is no employments in the list, else 1 */
int nextEmploymentOperation();

/* Operation: goes to next employment in list. Returns 0 if there is no departments in the list, else 1 */
int nextDepartmentOperation();

/* Operation: goes to previous employment in list. Returns 0 if there is no departments in the list, else 1.
 ATTENTION! It works with O(N) complexity, not O(1). */
int previousEmploymentOperation();

/* Operation: goes to previous employment in list. Returns 0 if there is no departments in the list, else 1.
 ATTENTION! It works with O(N) complexity, not O(1).*/
int previousDepartmentOperation();

/* Returns how many employments are in list in current department.
 * If something is unavailable, returns 0*/
size_t employmentsInDepartment();

/* Returns how many departments there is.
 * If something is unavailable, returns 0*/
size_t departmentsInCompany();

/* Cancels one actual (done or retried) operation
Returns 0 if something get wrong and 1 if success*/
int cancel();

/* Retries one cancelled operation.
Returns 0 if something get wrong and 1 if success*/
int retry();

/* Finalizes everything */
void finalization();

/* end processing of XML file */
void forgetOperationsHistory();

