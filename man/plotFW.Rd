\name{plot.FW}
\alias{plot.FW}
\title{
plot the FW object 
}
\description{
the plot.FW function plot the fitted object from calling the function \code{\link{FW}}.
}

\usage{
\method{plot}{FW}(x,plotVAR=NULL,chain=1,ENVlabel="split",\dots)
}
\arguments{
\item{x}{(class 'FW') returned from calling \code{\link{FW}} function.}
\item{plotVAR}{(character vector or integer vector) If plotVAR is a character vector, it specifies the variety identifiers for which the fitted values should be plotted. If plotVar is an integer vector, it specifies  the indexes of the varieties for which the fitted values should be plotted (for example, plotVAR=c(1:5) will plot the first five varieties). The default values is NULL and all the varieties will be plotted.
}

\item{chain}{(integer), only used for Gibbs method, when there are fitted values from multiple chains. This specifies from which chain the fitted value is to be plotted
}
\item{ENVlabel}{Whether to print environment labels and the label positions: available values are TRUE,FALSE,"split","top" or "bottom". If FALSE, environment labels will not be printed on x-axis. If TRUE or "split", half of the environment labels will be printed on the top of the plot and half of the environment labels will be printed on the bottom of the plot. The default is "split" so that if there are many environments, the labels can be displayed with more spaces. Otherwise, the environment labels will be printed at the bottom ("bottom") or at the top ("top") of the plot}
\item{\dots}{
other arguments to be passed for plot.FW and plot function (e.g., xlab, ylab, main).
}
}

\details{
The x-axis is the estimated values of environment effects. The y-axis is the variety performance. Each color represents a different variety. Lines are fitted values and circles are the cell means of genotype by environment combination.  
}
\author{
Lian Lian, Gustavo de los Campos
}
\examples{
data(wheat)
attach(wheat.Y)
lm2=FW(y=y,VAR=VAR,ENV=ENV,nIter=500,burnIn=200)
plot(lm2)
plot(lm2,plotVAR=c("1081265","1101307", "1295736", "13302", "1343502"))
}