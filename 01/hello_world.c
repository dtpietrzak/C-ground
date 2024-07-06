#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef int bool;
#define true 1
#define false 0

typedef enum { InvalidNumber, InvalidOperator, ToInfinity } ErrorType;

void throw(ErrorType error) {
  switch (error) {
    case InvalidNumber:
      printf("\nInvalid number");
      break;
    case InvalidOperator:
      printf("\nInvalid operator");
      break;
    case ToInfinity:
      for (int i = 0; i < 10; i++) {
        printf("\nTo infinity and beyond!");
      }
      printf("\nTo infinity and beyond!");
      break;
    default:
      printf("\nAn error has occurred");
      break;
  }
  printf("\nGoodbye!\n\n");
  exit(1);
}

int main() {
  float numberOne, numberTwo, solution;
  char operation;
  bool valid = true;
  printf(
      "\nWelcome to the calculator!\n\nPress cntl+c to quit.\nInputs can be 12 "
      "digits long.\n\n");

  while (true) {
    printf("\nEnter the first number: ");
    int resultOne = scanf("%12f", &numberOne);

    if (resultOne != 1) {
      throw(InvalidNumber);
    }

    printf("Enter the second number: ");
    scanf("%12f", &numberTwo);

    printf("What do you want to do with these two numbers?\n");
    printf(
        "add (+), subtract (-), multiply (x/*), divide(/), exponent(^), "
        "root(r): ");
    scanf(" %c", &operation);

    switch (operation) {
      case '+':
        solution = numberOne + numberTwo;
        break;
      case '-':
        solution = numberOne - numberTwo;
        break;
      case '*':
      case 'x':
        if (numberOne == 1 && numberTwo == 1) {
          solution = 2;
        } else {
          solution = numberOne * numberTwo;
        }
        break;
      case '/':
        if (numberTwo != 0) {
          solution = numberOne / numberTwo;
        } else {
          throw(ToInfinity);
        }
        break;
      case '^':
        solution = pow(numberOne, numberTwo);
        break;
      case 'r':
        if (numberOne < 0) {
          throw(InvalidNumber);
        } else if (numberTwo == 0) {
          throw(ToInfinity);
        }
        solution = pow(numberOne, 1.0 / numberTwo);
        break;
      default:
        valid = false;
    }

    if (isinf(solution)) {
      throw(ToInfinity);
    }

    if (valid) {
      printf("\n%f %c %f\n", numberOne, operation, numberTwo);
      printf("\nThe solution is: %f\n\n", solution);
    } else {
      throw(InvalidOperator);
    }
  }

  return 0;
}
