#include<stdio.h>
#define UNVISIT -1
#define VISIT  -2
#define NOISE -3
#define MAXNUMBER 10000
//this is called by rpair.c 
//the DBSCAN algorithm is implemented in this file
//this function only considers the one dimension data

void regionQuery(double data[], int index, int len, double eps, int result[])
{
  int i=index-1;
  for(;i>0;i--)
  {
	if(data[index]-data[i]>eps)
	 break;
  }
  result[0]=i+1;
  for(i=index+1;i<len;i++)
  {
	if(data[i]-data[index]>eps)
	 break;
  } 
//   printf("%lf, %lf\n", data[i], data[index]);
  result[1]=i-1;
}
void expandCluster(double  data[], int   mark[], int i, int len ,int  neighbors[], int c, double eps, int MinPts)
{
	 mark[i]=c;
	 int localnb[2];
	 int left=neighbors[0];
	 for(;left<=neighbors[1];left++)
	 {
		 if (mark[left]==UNVISIT)
		 {
			mark[left]=VISIT;
			regionQuery(data, left, len, eps, localnb);
			if(localnb[1]-localnb[0]>MinPts)
			{
				int temp=localnb[0];
				for(;temp<=localnb[1];temp++)
				{
					mark[temp]=c;
				}
			}
		 }
		 if (mark[left]<UNVISIT)
		 {
			mark[left]=c;
		 }
	 }
}
int DBSCAN(double  data[], int len, double eps, int MinPts,double record[][2])
{
int c=0;
int mark[MAXNUMBER];
int i;
int neighbors[2];
int cluster[1000];
int next_cluster=0;
for (i=0;i<len;i++)
{
mark[i]=UNVISIT;
}
for (i=0;i<len;i++)
{ 
  if(mark[i]==UNVISIT)
  {
  mark[i]=VISIT;
  regionQuery(data, i,len, eps, neighbors);
  if(neighbors[1]-neighbors[0]<MinPts)
	mark[i]=NOISE;
  else
    {
	   cluster[next_cluster++]=i;
	   expandCluster(data, mark, i, len, neighbors, i, eps, MinPts);
    }
  }
}
int j=0;
record[0][0]=0; record[0][1]=0;
for (i=0;i<len;i++)
{
  
  if(mark[i]!=cluster[j] && record[j][0]>0 && record[j][1]==0) 
  {
       record[j][1]=data[i-1];
       j++;
       record[j][0]=0; record[j][1]=0;
  }
  if(mark[i]==cluster[j] && record[j][0]==0) record[j][0]=data[i];
}
return next_cluster;
}

/*void sort(double a[], int len)
{
	int i, j;
	double temp;
	for(i=0;i<len;i++)
	{
		for(j=i+1;j<len;j++)
		{
			if(a[j]<a[i]) {temp=a[i];a[i]=a[j];a[j]=temp;}
		}
	}
}
int main()
{
double temp, a[MAXNUMBER];
int i=0;
FILE * fp=fopen("hi","r");
while(fscanf(fp,"%lf\n",&temp)!=EOF)
   a[i++]=temp;
fclose(fp);
sort(a,i);
//a={1.2,1.2,1.2,1.2,1.2,1.2,1.2,1.2,1.2,1.2};
DBSCAN(a, i, 0.5, 9);
return 0;
}*/
