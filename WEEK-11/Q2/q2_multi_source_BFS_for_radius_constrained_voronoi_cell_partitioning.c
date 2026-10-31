#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
static char ibuf[1<<16]; static size_t ipos=0, ilen=0;
static int gc(void){ if(ipos==ilen){ ilen=fread(ibuf,1,sizeof ibuf,stdin); ipos=0; if(!ilen) return -1;} return (unsigned char)ibuf[ipos++]; }
static int rd(long long *x){ int c=gc(); while(c!=-1&&c!='-'&&(c<'0'||c>'9')) c=gc(); if(c==-1) return 0; int s=1; if(c=='-'){s=-1;c=gc();} long long v=0; while(c>='0'&&c<='9'){ if(v>(LLONG_MAX-9)/10) v=(LLONG_MAX-9)/10; else v=v*10+(c-'0'); c=gc(); } *x=v*s; return 1; }
static int fail(const char *msg){ fprintf(stderr,"error: %s\n",msg); return 1; }
/* Input: n m K R / h_1..h_K (1-indexed) / m lines "u v".
   Output: n lines "owner dist" ("-1 -1" if unassigned). Exit 1 + stderr message on malformed input. */
int main(void){
    long long n,m,K,R,t;
    if(!rd(&n)||!rd(&m)||!rd(&K)||!rd(&R)) return fail("missing header");
    if(n<0||n>INT_MAX/4||m<0||m>INT_MAX/4||K<0||K>INT_MAX||R<0) return fail("header out of range");
    if(n==0&&K>0) return fail("hubs given but no vertices");
    int *hub=malloc((size_t)(K>0?K:1)*sizeof *hub);
    int *eu=malloc((size_t)(m>0?m:1)*sizeof *eu), *ev=malloc((size_t)(m>0?m:1)*sizeof *ev);
    int *start=calloc((size_t)n+2,sizeof *start), *adj=malloc((size_t)(2*m>0?2*m:1)*sizeof *adj);
    int *owner=malloc((size_t)(n>0?n:1)*sizeof *owner), *dist=malloc((size_t)(n>0?n:1)*sizeof *dist), *q=malloc((size_t)(n>0?n:1)*sizeof *q);
    int *pos=malloc((size_t)(n>0?n:1)*sizeof *pos);
    char *out=malloc((size_t)n*24+16);
    int rc=1;
    if(!hub||!eu||!ev||!start||!adj||!owner||!dist||!q||!pos||!out){ fail("out of memory"); goto done; }
    for(long long i=0;i<K;i++){
        if(!rd(&t)){ fail("truncated hub list"); goto done; }
        if(t<1||t>n){ fail("hub out of range"); goto done; }
        hub[i]=(int)(t-1);
    }
    for(long long i=0;i<m;i++){
        if(!rd(&t)){ fail("truncated edge list"); goto done; }
        if(t<1||t>n){ fail("edge endpoint out of range"); goto done; }
        eu[i]=(int)(t-1);
        if(!rd(&t)){ fail("truncated edge list"); goto done; }
        if(t<1||t>n){ fail("edge endpoint out of range"); goto done; }
        ev[i]=(int)(t-1);
        start[eu[i]+1]++; start[ev[i]+1]++;
    }
    for(long long i=0;i<n;i++) start[i+1]+=start[i];
    for(long long i=0;i<n;i++) pos[i]=start[i];
    for(long long i=0;i<m;i++){ adj[pos[eu[i]]++]=ev[i]; adj[pos[ev[i]]++]=eu[i]; }
    for(long long i=0;i<n;i++){ owner[i]=-1; dist[i]=-1; }
    long long qh=0,qt=0;
    /* hubs enqueued in index order => FIFO keeps owners sorted within each level => min-index tie-break */
    for(long long i=0;i<K;i++){ int h=hub[i]; if(owner[h]==-1){ owner[h]=(int)i+1; dist[h]=0; q[qt++]=h; } }
    while(qh<qt){
        int u=q[qh++];
        if(dist[u]>=R) continue;
        for(int j=start[u];j<start[u+1];j++){ int v=adj[j]; if(owner[v]==-1){ owner[v]=owner[u]; dist[v]=dist[u]+1; q[qt++]=v; } }
    }
    size_t o=0;
    for(long long i=0;i<n;i++) o+=(size_t)sprintf(out+o,"%d %d\n",owner[i],dist[i]);
    fwrite(out,1,o,stdout);
    rc=0;
done:
    free(out);free(pos);free(q);free(dist);free(owner);free(adj);free(start);free(ev);free(eu);free(hub);
    return rc;
}