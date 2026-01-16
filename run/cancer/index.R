library(dplyr)
library(ggplot2)
library(readr)
library(tidyr)
library(RColorBrewer)
library(ape)
setwd(".")
DATA <- read.csv('trajectory_copie.txt', header = F, sep = " ")
#DATA <- read.csv('/Users/thao_admin/Documents/Cancer_simuscale/cancer_simuscale/run/cancer/trajectory_1421746920 copie.txt', header = F, sep = " ")


DATA <-DATA[ -c(23) ]
colnames(DATA) <- c("TIME","N","ID","X","Y","Z","R","Type","status","TAG","S_S","S2","D_D","VOLUME","REMEMBER_DIVISION","CANCER_S","CANCER_D", "CANCER_P", "CANCER_mRNA_S", "CANCER_mRNA_D1", "CANCER_mRNA_P"," S2_D")

#print(DATA$TIME[length(DATA$TIME)])

### whole data
Day_0h <- filter(DATA, DATA$TIME == 0.00)
Day_420h <- filter(DATA, DATA$TIME == DATA$TIME[length(DATA$TIME)])


N_ <-group_by(Day_420h, TAG) %>%
  summarise(
    count = n(),
    mean = mean(X, na.rm = TRUE),
    sd = sd(X, na.rm = TRUE)
  )
#print(N_)
Percent <- N_$count[2]*100/(N_$count[2]+N_$count[1])
#print(Percent)

Day_420h <- filter(DATA, DATA$TIME == DATA$TIME[length(DATA$TIME)])

Max_x_at_fin <- max(Day_420h$X)
Min_x_at_fin <- min(Day_420h$X)
Max_y_at_fin <- max(Day_420h$Y)
Min_y_at_fin <- min(Day_420h$Y)
Max_z_at_fin <- max(Day_420h$Z)
Min_z_at_fin <- min(Day_420h$Z)

Z_10_pourcent <- (Max_z_at_fin-Min_z_at_fin)*0.1

Z_right_10 = Max_z_at_fin - Z_10_pourcent

Z_left_10 = Min_z_at_fin +Z_10_pourcent

Z_center = (Max_z_at_fin - Min_z_at_fin)/2. + Min_z_at_fin

Z_left_center = (Z_center - Z_left_10)/2 + Z_left_10 

Z_right_center = (Z_right_10 - Z_center)/2 + Z_center

z= Z_center

#error= 0.1
Ini_pos_420_Great <- filter (Day_420h, Day_420h$Z + Day_420h$R > z)#+ error )
Ini_pos_420_Z <- filter (Ini_pos_420_Great, Ini_pos_420_Great$Z - Ini_pos_420_Great$R < z)#-error )

Ini_pos_420_Z$Type[which(Ini_pos_420_Z$TAG == 1.)] <- 'Stem'
Ini_pos_420_Z$Type[which(Ini_pos_420_Z$TAG == 0.)] <- 'Synaptophisine'
Ini_pos_420_Z$Type <- as.factor(Ini_pos_420_Z$Type)


colors <- c( "red", "blue")
# 

colors <- colors[as.numeric(Ini_pos_420_Z$Type)]
# 
postscript(file="saving_plot4.ps")
# 
title = sprintf('Z = %s', z)
p2<-ggplot(data = Ini_pos_420_Z, mapping = aes(x = X, y = Y,color=as.factor(TAG))) + geom_point()+labs(color="Type") +#labs(size="R")+
  scale_color_manual(values= c( "red", "blue"),
                     breaks=c( "1", "0"),
                     labels= c( "Stem","Synaptophisine"))
p2 <- p2 + labs(title = title)+
  theme(plot.title = element_text(hjust = 0.5))
p2 <- p2 + xlab("X")
p2 <- p2 + ylab("Y")
#p<-p + theme(axis.title.y = element_text(size = 25), axis.title.x = element_text(size = 25),axis.text = element_text(size = 20), panel.grid.major = element_blank(), panel.grid.minor = element_blank(),
   #         panel.background = element_blank(), axis.line = element_line(colour = "black"))
p2 <-p2+ theme(legend.position="none")
p2
  
dev.off()
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

#print(G$count[G$TAG == 1])
P <-  max(G$count[G$TAG == 1],0)*100/( max(G$count[G$TAG == 1],0)+ max(G$count[G$TAG == 0],0))

P <- format(round(as.numeric(P), 1), nsmall = 1)
#print(P)

Pos <- max(G$count[G$TAG == 1],0)
#print(Pos)

Neg <- max(G$count[G$TAG == 0],0)
#print(Neg)
Dis_mean <-  max(G$mean[G$TAG == 0],0)- max(G$mean[G$TAG == 1],0)
#
Mean_P <- max(G$mean[G$TAG == 1],0)

Mean_N <- max(G$mean[G$TAG == 0],0)

X_random=as.character(Ini_pos_420_Z$TAG)
#print(X_random)

Sub.dists_random <- as.matrix(dist(cbind(Ini_pos_420_Z$X,Ini_pos_420_Z$Y)))

#min(Sub.dists_random[Sub.dists_random != 0])

#max(Sub.dists_random)

#R_max <- max(Ini_pos_420_Z$R)

Sub.dists.bin_random <- (Sub.dists_random > 0. & Sub.dists_random <= 1.5)

#print(sum(Ini_pos_420_Z$TAG))

if (Pos > 0 && Neg >0){
MI <- Moran.I(Ini_pos_420_Z$TAG, Sub.dists.bin_random)

I <- format(round(as.numeric(MI[1]), 3), nsmall = 3)

E <- format(round(as.numeric(MI[2]), 4), nsmall = 4)

Std <- format(round(as.numeric(MI[3]), 3), nsmall = 3)

P_val <- format(round(as.numeric(MI[4]), 1), nsmall = 1)
} else if (Pos == 0 || Neg ==0) {
  I <- 1}


#####entropy
Data_P <- filter(Ini_pos_420_Z, Ini_pos_420_Z$TAG == 1)

#print(Data_P)

print(dim(Data_P)[1])
if (dim(Data_P)[1] != 0) {

Dis_P <- as.matrix(dist(cbind(Data_P$X,Data_P$Y))) ## method =  "euclidean"
print(max(Dis_P))

hist_vec <- hist(Dis_P, breaks = 30)

Dx <- hist_vec$breaks[2]-hist_vec$breaks[1] # bin's width

Den <-  hist_vec$density

p_k <- Den*Dx

Ent_p <- -(sum(ifelse(P == 0, 0, p_k * log(p_k))))#+log(Dx)

Ent_p <- format(round(as.numeric(Ent_p), 3), nsmall = 3)

} else{
Ent_p <- 0.}

print(Ent_p)
Thre <- read.table('kineticsparam.txt', header = T, sep = "", dec =".")
#Thre <- read.csv('kineticsparam.txt', header = T, sep = "\t")
Th <- Thre['D_stem'] #Thre['V_SS']
Th2 <- Thre['S_stem']#Thre['V_D'] 
#Thre['S_stem']#Thre['V_SS'] #Thre['d1'] #Thre['D_stem']
#Th
#######

df <- data.frame(Threshold=c(Th),#V_SS=c(Th),#Threshold=c(Th),
                 Sig=c(Th2),#V_D=c(Th2),#Sig=c(Th2),
                 Pos=c(Pos),
                 Ne=c(Neg),
                 Mean_P=c(Mean_P),
                 Mean_N=c(Mean_N),
                 MI=c(I),
                 ENT=c(Ent_p),
                 Z=c(Z_center)) ### save the data

#df

write.table(df, file='data.txt', sep='\t')

#L <- read.csv('data.txt', header = T, sep = "\t")
#L['Z']+L['ENT']


