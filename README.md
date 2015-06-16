#GibbsFW: A Gibbs Sampler for Finlay-Wilkinson Regression with unbalanced data
Plant breeders use Finlay-Wilkinson Regression to assess the stableness different varieties across environments. The routine method used in Finlay-Wilkinson regression is regressing the observed phenotypic values on environment means. Variety effect and the rate of change of variety across the environments are solved by ordinary least squares. However, when the data is unbalanced, for example, when only some varieties are growing on some environments while other varieties are growing on the other sets of environments, the mean performance on each environment will not reflect the environment effect but will reflect the performance of the specific varieties grown on this environment. In this case, therefore, the ordinary least square method will not give accurate estimate of variety effect, the rate of change for varieties and environment effect.
We wrote this R package to simultaneously estimate the environment effect and the genetic effect by Gibbs Sampling.  


## Loading the package and data
```R
library(GibbsFW)
data(FWdat)
```

## Example 1 run Finlay-Wilkinson Regression by ordinary least squares
```R
lm1=lmFW(FWdat$y,VAR=FWdat$Variety, ENV=FWdat$Environment)

#the estimated values of b, g, h can be obtained as 
lm1$b
lm1$g
lm1$h

#the fitted values can be obtained as
lm1$fitted.values

#To plot the results
plot(lm1)

```

## Example 2 run Finlay-Wilkinson Regression by Gibbs Sampler with only one chain.

```R
lm2=GibbsFW(FWdat$y,VAR=FWdat$Variety, ENV=FWdat$Environment)

#the estimated values of b, g, h can be obtained as 
lm2$Init1$b
lm2$Init1$g
lm2$Init1$h

#the fitted values can be obtained as
lm2$Init1$fitted.values

#To plot the results of the first chain
plot(lm2$Init1)

#To plot only a few lines 
plot(lm2$Init1,plotVAR=c("V1","V4","V5","V9"))

```