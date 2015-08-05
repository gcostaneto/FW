#include <R.h>
#include <Rmath.h>
#include <Rinternals.h>
#include <Rdefines.h>
#include "sample_beta.h"
#include <stdlib.h>
#include <string.h>

//create a new string, which is the combination of two strings.
char* concat(const char *s1, char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
    //in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}


//main Gibbs sampler program.

SEXP C_GibbsFW(SEXP R_y, SEXP R_IDL, SEXP R_IDE, SEXP R_g, SEXP R_b, SEXP R_h, SEXP R_nIter, SEXP R_burnIn, SEXP R_thin, SEXP R_saveFile, SEXP R_S, SEXP R_Sg, SEXP R_Sb, SEXP R_Sh, SEXP R_df, SEXP R_dfg, SEXP R_dfb, SEXP R_dfh,SEXP R_var_e, SEXP R_var_g, SEXP R_var_b, SEXP R_var_h,SEXP R_mu, SEXP R_Ainv, SEXP R_whNA , SEXP R_whNotNA, SEXP R_VARstore, SEXP R_ENVstore){
    int nProtect=0;
    //data and initial values
    PROTECT(R_y=AS_NUMERIC(R_y));  nProtect+=1;
    PROTECT(R_IDL=AS_INTEGER(R_IDL)); nProtect+=1;
    PROTECT(R_IDE=AS_INTEGER(R_IDE)); nProtect+=1;
    PROTECT(R_g=AS_NUMERIC(R_g)); nProtect+=1;
    PROTECT(R_b=AS_NUMERIC(R_b)); nProtect+=1;
    PROTECT(R_h=AS_NUMERIC(R_h)); nProtect+=1;
    PROTECT(R_Ainv=AS_NUMERIC(R_Ainv)); nProtect+=1;
    PROTECT(R_whNA=AS_INTEGER(R_whNA)); nProtect+=1;
    PROTECT(R_VARstore=AS_INTEGER(R_VARstore)); nProtect+=1;
    PROTECT(R_ENVstore=AS_INTEGER(R_ENVstore)); nProtect+=1;   
    int i,j,k,l;
    int n= length(R_y);
    int ng=length(R_g);
    int nh=length(R_h);
    int nNa=length(R_whNA);
    int nVAR_Store=length(R_VARstore);
    int nENV_Store=length(R_ENVstore);
    double mu[1];mu[0]=NUMERIC_VALUE(R_mu);
    double var_e=NUMERIC_VALUE(R_var_e);
    double var_b=NUMERIC_VALUE(R_var_b);
    double var_g=NUMERIC_VALUE(R_var_g);
    double var_h=NUMERIC_VALUE(R_var_h);
    int nIter=INTEGER_VALUE(R_nIter);
    int burnIn=INTEGER_VALUE(R_burnIn);
    int thin=INTEGER_VALUE(R_thin);
    int nSamples=(nIter-burnIn)/thin;//burnIn is the number of discarded samples
    double *y=NUMERIC_POINTER(R_y);
    int *whNA=INTEGER_POINTER(R_whNA);
    int *IDL=INTEGER_POINTER(R_IDL);
    int *IDE=INTEGER_POINTER(R_IDE);
    double *Ainv=NUMERIC_POINTER(R_Ainv);
    int *VARstore=INTEGER_POINTER(R_VARstore);
    int *ENVstore=INTEGER_POINTER(R_ENVstore);
    
    int C_IDL[n];
    int C_IDE[n];
    for(i=0;i<n;i++){C_IDL[i]=IDL[i]-1; C_IDE[i]=IDE[i]-1;}
  
 

    
    //g, b, h are duplicates of R_g, R_b, R_h, they do not point to R_g, R_b, R_h, and do not modify them
    double *g=(double *) R_alloc(ng,sizeof(double));
    double *b=(double *) R_alloc(ng,sizeof(double));
    double *h=(double *) R_alloc(nh,sizeof(double));
    double *yhat=(double *) R_alloc(n,sizeof(double));
    double *yStar=(double *)R_alloc(n,sizeof(double));
    for(i=0;i<ng;i++){
        g[i]= NUMERIC_POINTER(R_g)[i];
        b[i]= NUMERIC_POINTER(R_b)[i];
    }
    for(i=0;i<nh;i++)h[i]=NUMERIC_POINTER(R_h)[i];
    for(i=0;i<n;i++){
    yhat[i]=mu[0];
    yStar[i]=y[i];
    }
    //starting value for yStar[whNA] is set to mu[0]
     if(nNa>0){
            for(j=0;j<nNa;j++){
                yStar[(whNA[j]-1)]=mu[0];
            }
        }
    
    
    // int nParameters=length(R_parameters);//will include this later so we can define parameter names to print out.
    double S=NUMERIC_VALUE(R_S);
    double Sg=NUMERIC_VALUE(R_Sg);
    double Sb=NUMERIC_VALUE(R_Sb);
    double Sh=NUMERIC_VALUE(R_Sh);
    double df=NUMERIC_VALUE(R_df);
    double dfg=NUMERIC_VALUE(R_dfg);
    double dfb=NUMERIC_VALUE(R_dfb);
    double dfh=NUMERIC_VALUE(R_dfh);
    double SS,DF;
    
    //saveFile
    FILE *fsaveFile = fopen(CHAR(STRING_ELT(R_saveFile,0)),"w");
    if (fsaveFile == NULL) error("Can't open input file !\n");
    //headers for samples file.
    fprintf(fsaveFile,"%s,%s,%s,%s,%s","mu","var_g","var_b","var_h","var_e");
           
            for(j=0;j<nVAR_Store;j++){
            	fprintf(fsaveFile,",g[%d]",VARstore[j]);
            }
            for(j=0;j<nVAR_Store;j++){
            	fprintf(fsaveFile,",b[%d]",VARstore[j]);
            }
            for(j=0;j<nENV_Store;j++){
            	fprintf(fsaveFile,",h[%d]",ENVstore[j]);
            }
            
    fprintf(fsaveFile,"\n");
            
        
    /************************************************
     * posteria and yhat storage
     ************************************************/
    SEXP R_post_mu,R_post_var_g,R_post_var_b,R_post_var_h,R_post_var_e,R_post_g,R_post_b,R_post_h, R_post_yhat;
    PROTECT(R_post_mu=allocVector(REALSXP,1)); nProtect+=1;
    PROTECT(R_post_var_g=allocVector(REALSXP,1)); nProtect+=1;
    PROTECT(R_post_var_b=allocVector(REALSXP,1));nProtect+=1;
    PROTECT(R_post_var_h=allocVector(REALSXP,1));nProtect+=1;
    PROTECT(R_post_var_e=allocVector(REALSXP,1));nProtect+=1;
    PROTECT(R_post_g=allocVector(REALSXP,ng));nProtect+=1;
    PROTECT(R_post_b=allocVector(REALSXP,ng));nProtect+=1;
    PROTECT(R_post_h=allocVector(REALSXP,nh));nProtect+=1;
    PROTECT(R_post_yhat=allocVector(REALSXP,n));nProtect+=1;
    
    double post_mu=0, post_var_e=0,post_var_g=0,post_var_b=0,post_var_h=0;
    double *post_g=NUMERIC_POINTER(R_post_g);
    double *post_b=NUMERIC_POINTER(R_post_b);
    double *post_h=NUMERIC_POINTER(R_post_h);
    double *post_yhat=NUMERIC_POINTER(R_post_yhat);
    //initialize to 0
    for(i=0;i<ng;i++){post_g[i]=0;post_b[i]=0;}
    for(i=0;i<nh;i++){post_h[i]=0;}
    for(i=0;i<n;i++){post_yhat[i]=0;}


    double *e=(double *) R_alloc(n,sizeof(double));
    double *X=(double *) R_alloc(n,sizeof(double));
   
    //*initial values for e.//yStar is y except for the NA values;
    for(j=0;j<n;j++) e[j]=yStar[j]-mu[0]-g[C_IDL[j]]-(1+b[C_IDL[j]])*(h[C_IDE[j]]);
    /************************************************
     * //begin Gibbs sampler
     ************************************************/
    
    GetRNGstate();
    int sampleCount=0;
    double tXX;
    double tXy;
    double C;
    double OmegaBeta;
    double betahat;
    double S2beta;
    for(i=0; i<nIter;i++){
        
        //sample environment effect h
        for(j=0;j<n;j++)X[j]=(1.0+b[C_IDL[j]]);
        sample_beta_ID(h,e,C_IDE,X,n,nh,var_e,var_h);
  
       
        //sample b and g 
        for(j=0;j<n;j++) X[j]=h[C_IDE[j]];
        for(k=0;k<ng;k++){
            tXy=0;
            tXX=0;
            //sample b
            for(j=0;j<n;j++){
                if(C_IDL[j]==k){
                    e[j]=e[j]+b[C_IDL[j]]*X[j];
                    tXX+=X[j]*X[j];
                    tXy+=X[j]*e[j];
                }
            }
            
            if(!ISNAN(Ainv[0])){
            	OmegaBeta=0;
                for(l=0;l<ng;l++){
                    if(l!=k){
                    	OmegaBeta+=Ainv[k+l*ng]*b[l];  
                    }
                }
                OmegaBeta=OmegaBeta/var_b;
                C=tXX/var_e+Ainv[k+k*ng]/var_b;
                betahat=(tXy/var_e-OmegaBeta)/C;
            }else{
            C=tXX/var_e+1/var_b;
            betahat=(tXy/var_e)/C;
            }
            
            b[k]=betahat+sqrtf(1/C)*norm_rand();
            
            for(j=0;j<n;j++){
                if(C_IDL[j]==k){
                    e[j]=e[j]-b[C_IDL[j]]*X[j];
                }
            }
            // sample g
            tXy=0;
            tXX=0;
            for(j=0;j<n;j++){
                if(C_IDL[j]==k){
                    e[j]=e[j]+g[C_IDL[j]];
                    tXX+=1;
                    tXy+=e[j];
                }
            }
            
            if(!ISNAN(Ainv[0])){
            	OmegaBeta=0;
                for(l=0;l<ng;l++){
                    if(l!=k){
                    	OmegaBeta+=Ainv[k+l*ng]*g[l]; 
                    }
                }
                OmegaBeta=OmegaBeta/var_g;
                C=tXX/var_e+Ainv[k+k*ng]/var_g;
                betahat=(tXy/var_e-OmegaBeta)/C;
            }else{
            	C=tXX/var_e+1/var_g;
                betahat=(tXy/var_e)/C;
            }
                g[k]=betahat+sqrtf(1/C)*norm_rand();
            
                for(j=0;j<n;j++){
                    if(C_IDL[j]==k){
                        e[j]=e[j]-g[C_IDL[j]];
                    }
                }
            }

        //var_e;
        SS=S;
        for(j=0;j<n;j++) SS+=e[j]*e[j];
        DF=n+df;
        var_e=SS/rchisq(DF);
        //var_h
        SS=Sh;
        for(j=0;j<nh;j++) SS+=h[j]*h[j];
        DF=nh+dfh;
        var_h=SS/rchisq(DF);
       
        //var_b;
        if(ISNAN(Ainv[0])){
            SS=Sb;
            for(j=0;j<ng;j++) SS+=b[j]*b[j];
            DF=ng+dfb;
            var_b=SS/rchisq(DF);
            //var_g;
            SS=Sg;
            for(j=0;j<ng;j++)SS+=g[j]*g[j];
            DF=ng+dfg;
            var_g=SS/rchisq(DF);
        }else{
        	SS=Sb;
        	S2beta=0;
        //only do calculations for the lower triangular since Ainv is symmetric
            for(j=1;j<ng;j++){
            	for(l=0;l<j;l++){
            	S2beta+=b[j]*Ainv[j+l*ng]*b[l];
            	}
            }
            S2beta=S2beta*2;
            for(j=0;j<ng;j++){
            S2beta+=b[j]*Ainv[j+j*ng]*b[j];
            } 
            SS=SS+S2beta;
            DF=ng+dfb;
            var_b=SS/rchisq(DF);
            //var_g;
            SS=Sg;
        	S2beta=0;
            for(j=1;j<ng;j++){
            	for(l=0;l<j;l++){
            	S2beta+=g[j]*Ainv[j+l*ng]*g[l];
            	}
            }
            S2beta=S2beta*2;
            for(j=0;j<ng;j++){
            S2beta+=g[j]*Ainv[j+j*ng]*g[j];
            } 
            SS=SS+S2beta;
            DF=ng+dfg;
            var_g=SS/rchisq(DF);
        }
       
        //sample intercept
        sample_mu(mu,e,var_e,n);
        
        //yhat & missing values
        for(j=0;j<n;j++){
            yhat[j]=yStar[j]-e[j];
        }
        if(nNa>0){
            for(j=0;j<nNa;j++){
                e[(whNA[j]-1)]=sqrtf(var_e)*norm_rand();
                yStar[(whNA[j]-1)]=yhat[(whNA[j]-1)]+e[(whNA[j]-1)];
            }
        }
        
        //running means and storing samples
        if(i>=(burnIn)){
            if((i+1)%thin==0){
            sampleCount+=1;
            post_mu += mu[0]/nSamples;
            post_var_e += var_e/nSamples;
            post_var_g += var_g/nSamples;
            post_var_b += var_b/nSamples;
            post_var_h += var_h/nSamples;
          
            
            for(j=0;j<nh;j++) {
            post_h[j] += h[j]/nSamples;
            }
            for(j=0;j<ng;j++) {
            post_b[j] += b[j]/nSamples;
            }
            
            //post_g
            for(j=0;j<ng;j++){
                post_g[j] += g[j]/nSamples;
            }
           //post_yhat
           for(j=0;j<n;j++){
        		post_yhat[j]+=yhat[j]/nSamples;
           }
      
      
         //store samples in file
         
            fprintf(fsaveFile,"%f,%f,%f,%f,%f",mu[0],var_g,var_b,var_h,var_e);
            
            for(j=0;j<nVAR_Store;j++){
            	fprintf(fsaveFile,",%f",g[(VARstore[j]-1)]);
            }
            
            for(j=0;j<nVAR_Store;j++){
            	fprintf(fsaveFile,",%f",b[(VARstore[j]-1)]);
            }
            for(j=0;j<nENV_Store;j++){
            	fprintf(fsaveFile,",%f",h[(ENVstore[j]-1)]);
            }
            fprintf(fsaveFile,"\n");
    	  }
        
        }//end of running means and storing samples.
      
     
            // print out iterations
        if((i+1)%1000==0){Rprintf("iter:%d\n",i+1);}
        
    }//end iteration
    
    //printout the number saved samples and number of samples expected to save
   // Rprintf("nSamples:%d, SampleCount:%d\n",nSamples,sampleCount);
    
 

    fclose(fsaveFile);

    PutRNGstate();//This write random numbers into R environments.

//return value to R

    REAL(R_post_mu)[0]=post_mu;
    REAL(R_post_var_g)[0]=post_var_g;
    REAL(R_post_var_b)[0]=post_var_b;
    REAL(R_post_var_h)[0]=post_var_h;
    REAL(R_post_var_e)[0]=post_var_e;
    

    SEXP list;
    
    
    PROTECT(list = allocVector(VECSXP, 9));nProtect+=1;
   
    SET_VECTOR_ELT(list, 0, R_post_mu);
    SET_VECTOR_ELT(list, 1, R_post_var_g);
    SET_VECTOR_ELT(list, 2, R_post_var_b);
    SET_VECTOR_ELT(list, 3, R_post_var_h);
    SET_VECTOR_ELT(list, 4, R_post_var_e);
    SET_VECTOR_ELT(list, 5, R_post_g);
    SET_VECTOR_ELT(list, 6, R_post_b);
    SET_VECTOR_ELT(list,7, R_post_h);
    SET_VECTOR_ELT(list,8, R_post_yhat);
 
    UNPROTECT(nProtect);
   

    return(list);

}




