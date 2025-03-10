library(dplyr)
library(ggplot2)
library(readr)
library(tidyr)
library(ape)
setwd(".")
DATA <- read.csv('trajectory_copie.txt', header = F, sep = " ")

DATA <-DATA[ -c(21) ]
colnames(DATA) <- c("TIME","N","ID","X","Y","Z","R","Type","TAG","S_S","VOLUME","REMEMBER_DIVISION","CANCER_S","CANCER_D",
                    "CANCER_D2", "CANCER_P", "CANCER_mRNA_S", "CANCER_mRNA_D1", "CANCER_mRNA_D2", "CANCER_mRNA_P")

### whole data
Day_0h <- filter(DATA, DATA$TIME == 0.00)
Day_420h <- filter(DATA, DATA$TIME == DATA$TIME[length(DATA$TIME)])


N_ <-group_by(Day_420h, TAG) %>%
  summarise(
    count = n(),
    mean = mean(X, na.rm = TRUE),
    sd = sd(X, na.rm = TRUE)
  )
Percent <- N_$count[2]*100/(N_$count[2]+N_$count[1])


### data at final time
Day_420h <- filter(DATA, DATA$TIME == DATA$TIME[length(DATA$TIME)])
Max_z_at_fin <- max(Day_420h$Z)
Min_z_at_fin <- min(Day_420h$Z)
Z_center = (Max_z_at_fin - Min_z_at_fin)/2. + Min_z_at_fin
z= Z_center

Ini_pos_420_Great <- filter (Day_420h, Day_420h$Z + Day_420h$R > z)
Ini_pos_420_Z <- filter (Ini_pos_420_Great, Ini_pos_420_Great$Z - Ini_pos_420_Great$R < z)

Ini_pos_420_Z$Type[which(Ini_pos_420_Z$TAG == 1.)] <- 'Stem'
Ini_pos_420_Z$Type[which(Ini_pos_420_Z$TAG == 0.)] <- 'Synaptophisine'
Ini_pos_420_Z$Type <- as.factor(Ini_pos_420_Z$Type)


Max_x <- max(Ini_pos_420_Z$X)
Min_x <- min(Ini_pos_420_Z$X)
Max_y <- max(Ini_pos_420_Z$Y)
Min_y <- min(Ini_pos_420_Z$Y)
center_x <- (Max_x - Min_x)/2. + Min_x
center_y <- (Max_y - Min_y)/2. + Min_y

Ini_pos_420_Z['Distance_center']=sqrt((Ini_pos_420_Z$X - center_x)^2+(Ini_pos_420_Z$Y - center_y)^2)

G <-group_by(Ini_pos_420_Z, TAG) %>%
  summarise(
    count = n(),
    mean = mean(Distance_center, na.rm = FALSE),
    sd = sd(Distance_center, na.rm = FALSE)
  )

##### percentage of stem cells

P <-  max(G$count[G$TAG == 1],0)*100/( max(G$count[G$TAG == 1],0)+ max(G$count[G$TAG == 0],0))

P <- format(round(as.numeric(P), 1), nsmall = 1)

Pos <- max(G$count[G$TAG == 1],0)
Neg <- max(G$count[G$TAG == 0],0)


#######Centrality = Mean_P/Mean_N
Mean_P <- max(G$mean[G$TAG == 1],0)
Mean_N <- max(G$mean[G$TAG == 0],0)

### Moran index
Sub.dists_random <- as.matrix(dist(cbind(Ini_pos_420_Z$X,Ini_pos_420_Z$Y)))

Sub.dists.bin_random <- (Sub.dists_random > 0. & Sub.dists_random <= 1.5)

if (Pos > 0 && Neg >0){
MI <- Moran.I(Ini_pos_420_Z$TAG, Sub.dists.bin_random)

I <- format(round(as.numeric(MI[1]), 3), nsmall = 3)

} else if (Pos == 0 || Neg ==0) {
  I <- 1}

#####entropy
Data_P <- filter(Ini_pos_420_Z, Ini_pos_420_Z$TAG == 1)

if (dim(Data_P)[1] != 0) {

Dis_P <- as.matrix(dist(cbind(Data_P$X,Data_P$Y))) ## method =  "euclidean"

hist_vec <- hist(Dis_P, breaks = 30)

Dx <- hist_vec$breaks[2]-hist_vec$breaks[1] # bin's width

Den <-  hist_vec$density

p_k <- Den*Dx

Ent_p <- -(sum(ifelse(P == 0, 0, p_k * log(p_k))))

Ent_p <- format(round(as.numeric(Ent_p), 3), nsmall = 3)

} else{
Ent_p <- 0.}

########## save the data
Thre <- read.table('kineticsparam.txt', header = T, sep = "", dec =".")
Th <- Thre['D_stem']
Th2 <- Thre['S_stem']

df <- data.frame(Threshold=c(Th),
                 Sig=c(Th2),
                 Pos=c(Pos),
                 Ne=c(Neg),
                 Mean_P=c(Mean_P),
                 Mean_N=c(Mean_N),
                 MI=c(I),
                 ENT=c(Ent_p),
                 Z=c(Z_center))

write.table(df, file='data.txt', sep='\t')



