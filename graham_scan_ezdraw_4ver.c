#include "ez-draw.h"
#include<math.h>
#include<time.h>

#define WIN_SIZE 720

struct point
{
    int x;
    int y;
};

typedef struct point Point;

float distance(Point a, Point b);
double cos_angle(Point a, Point b, Point c);
int cw_lin_ccw(Point a, Point b, Point c);
void mergesort(Point arr[], int L, int R);
void merge(Point arr[], int L, int mid, int R);

void win1_on_key_press (Ez_event *ev);
void win1_on_event (Ez_event *ev);
void win1_on_expose (Ez_event *ev);

void display(Point orig_arr[], int n, Point stack[], int top);


Point *gl_stack, *gl_orig_arr;
int gl_n, gl_top;

int main()
{
   
    int n;      // n = total number of points
    printf("Enter the number of points \n");
    scanf("%d", &n);
    Point arr[n];

    int choice;
    printf("Enter 0 to enter custom input, 1 to generate random input \n");
    scanf("%d", &choice);
    if(choice==0)
    {
        printf("Enter x and y coordinates of the points \n");
        printf("(Make sure that coordinates are between %d to %d, if you want the output to be represented correctly in the graph)\n", -WIN_SIZE/2, WIN_SIZE/2);
        for(int i=0; i<n; ++i)
        {
            scanf("%d%d", &arr[i].x, &arr[i].y);
        }
    }
    else
    {
        srand(time(0));
        for(int i=0; i<n; ++i)
        {
            arr[i].x = rand()%(WIN_SIZE-20) - WIN_SIZE/2 + 10;
            arr[i].y = rand()%(WIN_SIZE-20) - WIN_SIZE/2 + 10;
        }
    }


    //saving a copy of the original array, before we make changes to it
    Point orig_arr[n];
    for(int i=0; i<n; ++i)
    {
        orig_arr[i] = arr[i];
    }

    //ymin is the point with lowest y coordinate
    //incase there are multiple points with lowest y coordinate, pick the one with the lowest x coordinate
    //ymin_pos is the position of ymin in arr
    Point ymin = arr[0]; 
    int ymin_pos = 0;
    for(int i=1; i<n; ++i)
    {
        if( (arr[i].y < ymin.y) || ( (arr[i].y == ymin.y) && (arr[i].x < ymin.x)))
        {
            ymin_pos = i;
            ymin = arr[i];
        }
    }

    //swap ymin with arr[0] 
    Point temp = arr[0];
    arr[0] = arr[ymin_pos];
    arr[ymin_pos] = temp;

    //sort the points according to the angle it makes with arr[0] and x-axis
    mergesort(arr, 1, n-1);
    
    //if multiple points makes same angle with x-axis, keep the point which is farthest away from arr[0] and delete the rest
    int m = n;          //number of points
    Point x_axis;       //will be used when we calculate angle
    x_axis.y = arr[0].y;
    x_axis.x = arr[0].x + 1;
    for(int i=2; i<m; ++i)
    {
        while( fabs(cos_angle(arr[i], arr[0], x_axis) - cos_angle(arr[i-1], arr[0], x_axis)) < 0.000001 )
        {   
            --m;

            if(m<3 || i>=m)
            {
                break;
            }

            for(int j=i; j<m; ++j)
            {
                arr[j] = arr[j+1];
            }
        }
    }

    if(m<3)
    {
        printf("Convex Hull not possible \n");
        return 0;
    }

    Point stack[m];
    int top = -1;

    //push first 3 points into the stack
    stack[++top] = arr[0];
    stack[++top] = arr[1];
    stack[++top] = arr[2];

    for(int i=3; i<m; ++i)
    {
        while(top>=2 && cw_lin_ccw(stack[top-1], stack[top], arr[i]) <=0 )
        {
            --top;
        }
        stack[++top] = arr[i];
    }

    printf("All points = {");
    for(int i=0; i<n-1; ++i)
    {
        printf("(%d, %d), ", orig_arr[i].x, orig_arr[i].y);
    }
    printf("(%d, %d)} \n", orig_arr[n-1].x, orig_arr[n-1].y);

    printf("Convex Hull = {");
    for(int i=0; i<top; ++i)
    {
        printf("(%d, %d), ", stack[i].x, stack[i].y);
    }
    printf("(%d, %d)} \n", stack[top].x, stack[top].y);

    display(orig_arr, n, stack, top);
    
    return 0;
}

float distance(Point a, Point b)
{
    return sqrt( (b.x - a.x)*(b.x - a.x) + (b.y - a.y)*(b.y - a.y) );
}

double cos_angle(Point a, Point b, Point c)
{
    double ba_dot_bc = (b.x-a.x)*(b.x-c.x) + (b.y-a.y)*(b.y-c.y);
    return ba_dot_bc/(distance(b, a)*distance(b, c));
}

int cw_lin_ccw(Point a, Point b, Point c)
{
    //this function takes 3 points - A, B, C as inputs 
    //returns -1 if ABC are clockwise, 0 if they are collinear, 1 if they are counter clockwise
    int signed_area_2 = (b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x);
    //this gives the twice the signed area of the triangle ABC

    if(signed_area_2 < 0)
        return -1;       //A, B, C are clockwise if signed area of triangle ABC is negative
    else if(signed_area_2 == 0)
        return 0;       //A, B, C are collinear if triangle ABC has zero area
    else
        return 1;       //A, B, C are counter clockwise if signed area of triangle ABC is positive
}

void mergesort(Point arr[], int L, int R)
{
    if(L >= R)
        return;
    
    int mid = (L + R)/2;
    mergesort(arr, L, mid);
    mergesort(arr, mid+1, R);
    merge(arr, L, mid, R);
}

void merge(Point arr[], int L, int mid, int R)
{
    int L_n = mid - L + 1;
    int R_n = R - mid;

    Point L_arr[L_n];
    for(int i=0; i<L_n; ++i)
    {
        L_arr[i] = arr[L + i];
    }

    Point R_arr[R_n];
    for(int i=0; i<R_n; ++i)
    {
        R_arr[i] = arr[mid+1+i];
    }

    Point x_axis;
    x_axis.y = arr[0].y;
    x_axis.x = arr[0].x + 1;
    int i=0, j=0, k=0;

    while(i+j < L_n + R_n)
    {
        if(i==L_n)
            arr[L + k++] = R_arr[j++];
        else if(j==R_n)
            arr[L + k++] = L_arr[i++];
        else if(cos_angle(L_arr[i], arr[0], x_axis) - cos_angle(R_arr[j], arr[0], x_axis) > 0.000001)
            arr[L + k++] = L_arr[i++];
        else if(fabs(cos_angle(L_arr[i], arr[0], x_axis) - cos_angle(R_arr[j], arr[0], x_axis)) < 0.000001 && (distance(L_arr[i], arr[0]) > distance(R_arr[j], arr[0])))
            arr[L + k++] = L_arr[i++];
        else
            arr[L + k++] = R_arr[j++];
    }
}

void display(Point orig_arr[], int n, Point stack[], int top)
{
    gl_top = top;
    gl_n = n;
    
    gl_orig_arr = orig_arr;
    gl_stack = stack;

    if (ez_init() < 0) exit(1);
    ez_window_create (WIN_SIZE, WIN_SIZE, "Graham Scan Graph Window", win1_on_event);
    ez_main_loop ();
    exit(0); 
}

void win1_on_key_press(Ez_event *ev)
{
    switch(ev->key_sym)
    {
        case XK_q: 
            ez_quit(); 
            break;
    }

}

void win1_on_event(Ez_event *ev)    //called by ez_main_loop()
{
    switch(ev->type) 
    {
        case Expose: 
            win1_on_expose(ev); 
            break;
        case KeyPress: 
            win1_on_key_press(ev); 
            break;
    }
}

void win1_on_expose (Ez_event *ev)
{
    int shift = WIN_SIZE/2;

    ez_set_color(ez_grey);
    ez_set_thick(1);
    ez_draw_line(ev->win, shift, 2*shift, shift, 0);
    ez_draw_line(ev->win, 2*shift, shift, 0, shift);

    ez_set_color(ez_black);
    ez_set_thick(3);

    for(int i=0; i<gl_top; ++i)
    {
        ez_draw_line(ev->win, gl_stack[i].x + shift, -gl_stack[i].y + shift, gl_stack[i+1].x + shift, -gl_stack[i+1].y + shift);
    }
    ez_draw_line(ev->win, gl_stack[gl_top].x + shift, -gl_stack[gl_top].y + shift, gl_stack[0].x + shift, -gl_stack[0].y + shift);

    ez_set_color(ez_red);
    ez_set_thick(6);

    for(int i=0; i<gl_n; ++i)
    {
        ez_draw_point(ev->win,  gl_orig_arr[i].x + shift, -gl_orig_arr[i].y + shift);
    }
}
