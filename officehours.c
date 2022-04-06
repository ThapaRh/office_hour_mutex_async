//Rhitu Thapa
//1001751843


#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

/*** Constants that define parameters of the simulation ***/

#define MAX_SEATS 3        /* Number of seats in the professor's office */
#define professor_LIMIT 10 /* Number of students the professor can help before he needs a break */
#define MAX_STUDENTS 1000  /* Maximum number of students in the simulation */

#define CLASSA 0
#define CLASSB 1
#define CLASSC 2
#define CLASSD 3
#define CLASSE 4

/* TODO */
/* Add your synchronization variables here */

 pthread_mutex_t lock;
 pthread_cond_t leaving_students;
 pthread_cond_t professor_in_office;

 static int students_in_office;   /* Total numbers of students currently in the office */
 static int classa_inoffice;      /* Total numbers of students from class A currently in the office */
 static int classb_inoffice;      /* Total numbers of students from class B in the office */
 static int students_since_break = 0;


static int con_a;//to store the consecutive number of a class students
static int con_b;//to store consecutive number of b class students
static int rem_a;//remaining students in office from a class
static int rem_b;//remaining students in class from b
static int is_off;//to say if prof is off or not


typedef struct
{
  int arrival_time;  // time between the arrival of this student and the previous student
  int question_time; // time the student needs to spend with the professor
  int student_id;
  int class;
} student_info;

/* Called at beginning of simulation.
 * TODO: Create/initialize all synchronization
 * variables and other global variables that you add.
 */
static int initialize(student_info *si, char *filename)
{
  students_in_office = 0;
  classa_inoffice = 0;
  classb_inoffice = 0;
  students_since_break = 0;
  con_a = 0;
  con_b=0;
  rem_a=0;
  rem_b=0;
  is_off=1;

   pthread_mutex_init(&lock, NULL);//used to lock and unlock the critical region
   pthread_cond_init(&professor_in_office, NULL);//to figure out the vailability of professor
   pthread_cond_init(&leaving_students, NULL);//to figure out if there are any students in office


  /* Read in the data file and initialize the student array */
  FILE *fp;

  if((fp=fopen(filename, "r")) == NULL)
  {
    printf("Cannot open input file %s for reading.\n", filename);
    exit(1);
  }

  int i = 0;
  while ( (fscanf(fp, "%d%d%d\n", &(si[i].class), &(si[i].arrival_time), &(si[i].question_time))!=EOF) &&
           i < MAX_STUDENTS )
  {
    i++;
  }

 fclose(fp);
 return i;
}

/* Code executed by professor to simulate taking a break
 * You do not need to add anything here.
 */
static void take_break()
{
  printf("The professor is taking a break now.\n");
  sleep(5);
  assert( students_in_office == 0 );
  students_since_break = 0;
}

/* Code for the professor thread. This is fully implemented except for synchronization
 * with the students.  See the comments within the function for details.
 */
void *professorthread(void *junk)
{
  printf("The professor arrived and is starting his office hours\n");

  /* Loop while waiting for students to arrive. */
  while (1)
  {
    pthread_mutex_lock(&lock);//locks th ecritical region and waits till the professor is in break if he actually is on a break
    if (is_off){
      is_off = 0;
      pthread_cond_broadcast(&professor_in_office);
    }

    if(students_in_office==0 && students_since_break==professor_LIMIT)//checks if the max students after break is 10, and no students in class if so then professor goes on a break
    {
      is_off=1;
      take_break();
      is_off = 0;
      pthread_cond_broadcast(&professor_in_office); //this will wait until professor is on a break and when he comes out of his break then the signal is send to aleart stduenst athat professor if back
    }
    pthread_mutex_unlock(&lock);
  }
  pthread_exit(NULL);
}


/* Code executed by a class A student to enter the office.
 * You have to implement this.  Do not delete the assert() statements,
 * but feel free to add your own.
 */
void classa_enter()
{

  /* TODO */
  /* Request permission to enter the office.  You might also want to add  */
  /* synchronization for the simulations variables below                  */
  /*  YOUR CODE HERE.   */
  pthread_mutex_lock(&lock);
  rem_a++;

  while(1)
  {
    while((students_since_break>=professor_LIMIT) || is_off)//this will make the students wait until the professor is on a break and the signal is sent after the professor is back from the break.
    {
      pthread_cond_wait(&professor_in_office,&lock);
    }
    while(!((classb_inoffice==0)&&(students_in_office<MAX_SEATS)&&(con_a < 5 || rem_b ==0)))//it checks the condition if the class b students in office is 0 cause both class a and class b students cannot be in the office at the same time.
    {
      pthread_cond_wait(&leaving_students,&lock);//if the condition is true then it will make the alternate class students to wait
    }
    if (!(students_since_break>=professor_LIMIT || is_off) && ((classb_inoffice==0)&&(students_in_office<MAX_SEATS)&&(con_a < 5 || rem_b == 0)))//and if this condition is satisfied then now we start allowing the class a students into the professors office hours
    {
      students_in_office+=1;
      students_since_break+=1;
      classa_inoffice+=1;

      con_b = 0;
      con_a++;
      rem_a--;
      break;
    }
  }
  pthread_mutex_unlock(&lock);

}

/* Code executed by a class B student to enter the office.
 * You have to implement this.  Do not delete the assert() statements,
 * but feel free to add your own.
 */
void classb_enter()
{

  /* TODO */
  /* Request permission to enter the office.  You might also want to add  */
  /* synchronization for the simulations variables below                  */
  /*  YOUR CODE HERE. */
  pthread_mutex_lock(&lock);
  rem_b++;

  while(1)
  {
    while((students_since_break>=professor_LIMIT) || is_off)//if the professor is on a break then students are made to wait els they can enter
    {
      pthread_cond_wait(&professor_in_office,&lock);
    }
    while(!((classa_inoffice==0)&&(students_in_office<MAX_SEATS)&&(con_b < 5 || rem_a ==0)))//checks if the class a students are still in office if they are then class b studentss are made to wait till all ofthe class a students are done with their questions
    {
      pthread_cond_wait(&leaving_students,&lock);
    }
    if ((!(students_since_break>=professor_LIMIT || is_off)) && ((classa_inoffice==0)&&(students_in_office<MAX_SEATS)&&(con_b < 5 || rem_a ==0)))//if there are no class a students and professor is not in the break either then the admission is made for the class b students into the office hours
    {
      students_in_office+=1;
      students_since_break+=1;
      classb_inoffice+=1;

      con_a = 0;
      con_b++;
      rem_b--;
      break;
    }
  }
  pthread_mutex_unlock(&lock);

}

/* Code executed by a student to simulate the time he spends in the office asking questions
 * You do not need to add anything here.
 */
static void ask_questions(int t)
{
  sleep(t);
}


/* Code executed by a class A student when leaving the office.
 * You need to implement this.  Do not delete the assert() statements,
 * but feel free to add as many of your own as you like.
 */
static void classa_leave()
{
  /*
   *  TODO
   *  YOUR CODE HERE.
   */
  pthread_mutex_lock(&lock);
  students_in_office -= 1;
  classa_inoffice -= 1;

  pthread_cond_broadcast(&leaving_students);
  pthread_mutex_unlock(&lock);

}

/* Code executed by a class B student when leaving the office.
 * You need to implement this.  Do not delete the assert() statements,
 * but feel free to add as many of your own as you like.
 */
static void classb_leave()
{
  /*
   * TODO
   * YOUR CODE HERE.
   */
   pthread_mutex_lock(&lock);
   students_in_office -= 1;
   classb_inoffice -= 1;

   pthread_cond_broadcast(&leaving_students);
   pthread_mutex_unlock(&lock);
}

/* Main code for class A student threads.
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void* classa_student(void *si)
{
  student_info *s_info = (student_info*)si;

  /* enter office */
  classa_enter();

  printf("Student %d from class A enters the office\n", s_info->student_id);

  assert(students_since_break <= professor_LIMIT);
  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classb_inoffice == 0 );

  /* ask questions  --- do not make changes to the 3 lines below*/
  printf("Student %d from class A starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class A finishes asking questions and prepares to leave\n", s_info->student_id);

  /* leave office */
  classa_leave();

  printf("Student %d from class A leaves the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* Main code for class B student threads.
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void* classb_student(void *si)
{
  student_info *s_info = (student_info*)si;

  /* enter office */
  classb_enter();

  printf("Student %d from class B enters the office\n", s_info->student_id);
  assert(students_since_break <= professor_LIMIT);
  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classa_inoffice == 0 );

  printf("Student %d from class B starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class B finishes asking questions and prepares to leave\n", s_info->student_id);

  /* leave office */
  classb_leave();

  printf("Student %d from class B leaves the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* Main function sets up simulation and prints report
 * at the end.
 * GUID: 355F4066-DA3E-4F74-9656-EF8097FBC985
 */
int main(int nargs, char **args)
{
  int i;
  int result;
  int student_type;
  int num_students;
  void *status;
  pthread_t professor_tid;
  pthread_t student_tid[MAX_STUDENTS];
  student_info s_info[MAX_STUDENTS];

  if (nargs != 2)
  {
    printf("Usage: officehour <name of inputfile>\n");
    return EINVAL;
  }

  num_students = initialize(s_info, args[1]);
  if (num_students > MAX_STUDENTS || num_students <= 0)
  {
    printf("Error:  Bad number of student threads. "
           "Maybe there was a problem with your input file?\n");
    return 1;
  }

  printf("Starting officehour simulation with %d students ...\n",
    num_students);

  result = pthread_create(&professor_tid, NULL, professorthread, NULL);

  if (result)
  {
    printf("officehour:  pthread_create failed for professor: %s\n", strerror(result));
    exit(1);
  }

  for (i=0; i < num_students; i++)
  {

    s_info[i].student_id = i;
    sleep(s_info[i].arrival_time);

    student_type = random() % 2;

    if (s_info[i].class == CLASSA)
    {
      result = pthread_create(&student_tid[i], NULL, classa_student, (void *)&s_info[i]);
    }
    else // student_type == CLASSB
    {
      result = pthread_create(&student_tid[i], NULL, classb_student, (void *)&s_info[i]);
    }

    if (result)
    {
      printf("officehour: thread_fork failed for student %d: %s\n",
            i, strerror(result));
      exit(1);
    }
  }

  /* wait for all student threads to finish */
  for (i = 0; i < num_students; i++)
  {
    pthread_join(student_tid[i], &status);
  }

  /* tell the professor to finish. */
  pthread_cancel(professor_tid);

  printf("Office hour simulation done.\n");

  return 0;
}
