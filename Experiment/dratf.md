"""
The proposed (A NEW SETTING NAME?) is a task which aims to generate pixel level class segmentation mask
base on only one train sample with class label per category. Different from [1] which 
need other images with weaklier label that provide the number of object categories included in each sample.
we only use former images for training. This task can be view as a combination of one-shot and image level WSSS,
which only use class label and a few images as support images but no more other images for training. The main differences
between this task and aboves is:
1). Compare to image level level WSSS, there is no enough category labeling provided for network training or other
weaklier labels provided.
2). Compare to one-shot learning, there is only one image in train dataset and support dataset but latter has lots of 
images in train dataset. 
Inspired by DINO, we can see that pretrained parameters has rich priori knowledge, so the basic idea 
of our method is to take advantage of object's information from image level to pixel level.
Similar to few-shot learning, our fundamental idea is to guide segmentation process by effectively
incorporating the pixel level similarity between prototypes and test samples. We first extracted 
the high-level feature from backbone and generate prototype for each images. Note each image in training set
only has one class so that this prototype can represent this class. If there are more than one class
in each image, it is may confused for our method to predicting the class label of each pixel.
Usually, the high-level features between same class object embedded by backbone will closer.
Therefore, we propose to embedding the train samples to representative vectors and view them as
prototype, which is from high-level feature. Then, we get the similarity matrix by calculating 
cosine similarities between the representative vectors and the features of query images at
each pixel when testing. The value will be high if a pixel and a prototype is same class since
the feature vectors corresponding to the pixels of objects extracted from backbone in query image
is close to the prototype if they are same class. Otherwise, the score will be low. The similarities
will be used to generate final semantic segmentation map.
Our contributions are summarized as follows:
1).
2).
3). ???




4 EXPERIMENTS
4.1 Datasets and Evaluation Metric:
We establish our (NEW SETTING NAME?) task and evaluate our method on PASCAL VOC 2012 image segmentation
benchmark[]. For PASCAL VOC 2012 dataset, there are 1464 images for training, 1449 images for validating
and 1456 images for testing with 20 foreground and 1 background. In our task, we simply choose 20 images   
from training dataset. Note that only one sample with image level category label is provided per class.
We evaluate the performance of our method on validation set and mean intersection-over-union
(mIoU) is adopted as the evaluation metric. During training, not any class label is used and during inference, 
the class label is used for predicting pixel-wise class for images.

4.2 Implementation Details:
We choose Visual Transformer with small scale patch16 as our backbone, the RGB image is normalized
before inputting to the network. We use the weights pretrained on ImageNet-22K []
to initialize the model of Visual Transformer. During training, we simply resize the input image to 384x384.
Our method do not need any GPU.

5. Conclusion
Currently, deep learning approaches have achieved top performance in many computer vision problems.
However, predicting the class label of each pixel is still a very challenging task. In this paper, 
we have proposed a new task(NEW SETTING NAME?), which is more challenge than WSSS and one-shot
semantic segmentation. We only use one sample per class as our train dataset without any other data or annotation.
// TODO: add method conclusion
The proposed method can apply without any GPU.





3. Method


3.1 Problem Definition

For a dataset with C object categories, training dataset
$L_{train}$ = {$N_i$ | i ${\subset}$ C} images contain single category
of objects. Inspired by CAM[] model which pretrained in ImageNet is used to extract
high-level feature which can used for generating prototype for each class. In our
experiments, ViT[] is utilized as backbone. Additional, one sample that only contains
one class object is selected for each class c(c = 1, 2, ..., C). For inference, we
will generate a segmentation mask for each image such as fully-supervised semantic segmentation.

3.2 Overview of Our Approach
3.2.1 


In this section, we firstly feed input images to backbone and extract high-level feature for
generating prototypes. Then, for each query image, the similarity matrix will be calculated
between these prototypes and pixel-level features which extracted from query image. This
similarity matrix will be used to combine the high-level feature and prototypes and
predict the pixel-level segmentation mask. Our framework is shown as Fig1.
    
\subsection{Prototype}

prototype is supposed to be a class representation which is close to the high-level
features embedded by backbone from query image so that it can be used as a guidance for classification task. These are two alternative way to generator prototypes from high-level 
feature with backbone. 1). Use masked average pooling operation for foreground pixel. OSLSM [] proposes to remove the background pixels images and apply average pooling operation. However, statistic distribution may be changed if we remove the background pixel of input. So we can remove background pixel in high-level feature 
room. 2). Simply use class token as our prototype. Since transformer use attention mechanism to aggregate the feature and automatically adjust the weight between query and key. However, average pooling will give the same weights to all key which limit the expression ability of model. 
In our method, we use second way to generate prototype.

\subsection{Similarity Guidance Method}


As shown in Fig1. 
Assume $P_c$ $\subset$ $\mathbb{R}^{1,c}$ is the prototype of class $c$, we define $P$ $=$ {$P_i$ $\mid$ $i$ $\subset$ $C$}.
Then the similarity matrix:

$S = \frac{P \cdot F}{\left \| P \cdot F \right \|}$

where the $F$ $\subset$ $\mathbb{R}^{P, C}$ is the high-level feature, $P$ is the patch size after transformer embedding. Only use similarity matrix to predict pixel class is not enough since each prototype
is generated by one sample which is randomly selected in dataset and may not represent the whole class. To overcome this issue, we introduce two class score for improve performance. 1). Similar to some CAM methods[], we simply calculate $CAM_{score}$:
 
$CAM_{score}^c = \frac{1}{n} \sum_i^n{S_i^c \cdot M_i}$

While $S_i$ is the $i^{th}$ position of similarity matrix and $M_i$ is $i^{th}$ pixel of saliency mask $M$ created by TokenCut[]. $c$ is the $c^{th}$ category of $C$.And $n$ is the number of foreground pixel in $M$. 2). Considering the particularity of Vit, to make full use of information
from out backbone, we introduce $CLS_{score}$:

$CLS_{score}^c = D\left(P_c,CLS_{query}\right)$

While $D$ is $L_2$ distance and $P_c$ is the prototype of class $c$ and $CLS_{query}$ is the class token of query image.
The final class score $Class_{score}$:

$Class_{score}^c = CAM_{score}^c \cdot Norm\left(1 - CLS_{score}^c \right)$  

While $Norm$ is min-max normalization. Hence, if class $c$ is in query image, the $Class_{score}^c$ will be high.

Finally, the class activate map and segmap can be calculated as:

$CAM = argmax\left( Class_{score}^c \right)$

$
\begin{equation}
\label{eq6}
[x_{i}]=\left\{
\begin{aligned}
x_{ac} & , & \mu_{a}(x_{i})\geq \mu_{b}(x_{i}), \\
x_{bc} & , & \mu_{a}(x_{i})< \mu_{b}(x_{i}).
\end{aligned}
\right.
\end{equation}
$ 








3.2 Overview
Fig.1 shows the overall framework of our approach.




















[1]: Weaklier Supervised Semantic Segmentation With Only One Image Level Annotation per Category
[2]: A. Shaban, S. Bansal, Z. Liu, I. Essa, and B. Boots, “One-shot learning
for semantic segmentation,” BMVC, 2017. 1, 2, 4, 5, 6, 7, 8, 9
"""


\documentclass{article}
\usepackage[utf8]{inputenc}
\usepackage[english]{babel}

\usepackage[
backend=biber,
style=alphabetic,
sorting=ynt
]{biblatex}

\addbibresource{mybibliography.bib}

\title{Bibliography management: \texttt{biblatex} package}
\author{Share\LaTeX}
\date{ }

\begin{document}

\maketitle

\section{First section}

Using \texttt{biblatex} you can display bibliography divided into sections,
depending of citation type. 
Let's cite! The Einstein's journal paper \cite{einstein} and the Dirac's 
book \cite{dirac} are physics related items. 
Next, \textit{The \LaTeX\ Companion} book \cite{latexcompanion}, the 
Donald Knuth's website \cite{knuthwebsite}, \textit{The Comprehensive 
Tex Archive Network} (CTAN) \cite{ctan} are \LaTeX\ related items; but 
the others Donald Knuth's items \cite{knuth-fa,knuth-acp} are dedicated 
to programming. 

\medskip

\printbibliography
\end{document}

Carl Doersch, Abhinav Gupta, and Alexei A Efros. Unsupervised visual representation learning by context prediction.
In Proc. IEEE International Conference on Computer Vision
(ICCV), 2015.

Spyros Gidaris, Praveer Singh, and Nikos Komodakis. Unsupervised representation learning by predicting image rotations. arXiv preprint arXiv:1803.07728, 2018.

Deepak Pathak, Philipp Krahenbuhl, Jeff Donahue, Trevor
Darrell, and Alexei A Efros. Context encoders: Feature
learning by inpainting. In Proc. IEEE Conference on Computer Vision and Pattern Recognition (CVPR), 2016




Gustav Larsson, Michael Maire, and Gregory
Shakhnarovich. Learning representations for automatic
colorization. In Proc. European Conference on Computer
Vision (ECCV), 2016.

Ian Goodfellow, Jean Pouget-Abadie, Mehdi Mirza, Bing
Xu, David Warde-Farley, Sherjil Ozair, Aaron Courville, and
Yoshua Bengio. Generative adversarial nets. In Proc. Neural
Information Processing Systems (NIPS), 2014




\subsection{Prototype}

prototype is supposed to be a class representation which is close to the high-level
features embedded by backbone from query image so that it can be used as a guidance for classification task. For a RGB image $I \subset \mathbb{R}^3$, we use Vit as our backbone and embedding the image to $F \subset \mathbb{R}^{\left[CLS+HW, C\right]}$.
We divide $F$ into two parts: patch feature $F_{patch} \subset \mathbb{R}^{\left[HW, C\right]}$ and class token feature $F_{cls} \subset \mathbb{R}^{\left[CLS, C\right]}$. Where $HW$ is the height and width of feature and $C$ is the channel dimensions. 
Obviously, these are two alternative ways to generate prototypes from high-level feature $F$. 1). Use masked average pooling operation for foreground pixel. Shaban et al. \cite{shaban2017one} proposes to remove the background pixels images and apply average pooling operation. However, statistic distribution may be changed if we remove the background pixel of input. So we can remove background pixel in high-level feature room. We define the $local \ prototype$ of the $c^{th}$ sample as:

\begin{center}

$P_{local}^c = \frac{1}{n} \sum_i^n F_i^c \cdot M_i^c$

$P_{local} = \{ P_{local}^c \mid c \subset N_{total} \}$

\end{center}

while $F_i^c$ is the $i^{th}$ patch of $F_{patch}$, $N_{total}$ is the total categories of our samples. $M_i$ is the $i^{th}$ pixel of 
binary mask $M$ which indicate the foreground and background of $I$ and $n$ is the number of foreground pixel. 2). Simply use $F_{cls}$ as our $global \ prototype$ $P_{global}$ as follow:

\begin{center}

$P_{global} = F_{cls}$

\end{center}

Since transformer use attention mechanism to aggregate the feature and automatically adjust the weight between query and key. 

\subsection{Similarity Guidance Method}

Give a query image $I_{q} \subset \mathbb{R^3}$, the feature after embedding is $F_q \subset \mathbb{R}^{\left[CLS+HW, C\right]}$. We divide feature into two parts:
$F_{qcls} \subset \mathbb{R}^{\left[CLS, C\right]}$ and $F_{qpatch} \subset \mathbb{R}^{\left[HW, C\right]}$. We define local score map $M_{local} \subset \mathbb{R}^{\left[HW, N\right]}$ as our similarity matrix:

\begin{center}
$M_{local} = ReLU\left( Norm\left(F_{qpatch}\right) \cdot P_{local}^\mathrm{T} \right)$
\end{center}

Where $Norm$ is L2 normalization, and $ReLU$ is activation function which used to
remove negative score. Whereas, only use $M_{local}$ to predict pixel class is not enough since each prototype is generated by one sample which is randomly selected in dataset and may not represent the whole class. To overcome this issue, we introduce global score $S_g$ for improving performance:

\begin{center}
$Sg = Softmax\left(\mu\left(Norm\left(F_{qcls}\right) \cdot P_{global}\right)\right)$
\end{center}

Where $\mu$ is a weight factor. Finally, the class activate map and segmap can be calculated as:

\begin{center}
$CAM = Sg \cdot M_{local}$
\end{center}

\begin{center}
$
SegMap_i=
\begin{cases}
  & 0 \hspace{5em} if \quad Sal_i==1  \\
  & CAM_i \hspace{4em} otherwise 
\end{cases}
$
\end{center}

Where $CAM_i$ is $i^{th}$ position of $CAM$ and $Sal_i$ is $i_{th}$ pixel of the saliency map generated by unsupervised method TokenCut.


\section{Conclusion}

Currently, deep learning approaches have achieved top performance in many computer vision problems.
However, predicting the class label of each pixel is still a very challenging task. In this paper, 
we have proposed a new task(NEW SETTING NAME?), which is more challenge than WSSS and one-shot
semantic segmentation. We only use one sample per class as our train dataset without any other data or annotation.
We simply generate local and global prototypes from training samples and calculate the similarity between prototypes and query image features. Then we use global score $S_g$ to remove the objects categories which not exist in query image. Our proposed method can apply without any GPU.


Abstract—One-shot image semantic segmentation poses a challenging task of recognizing the object regions from unseen
categories with only one annotated example as supervision. In
this paper, we propose a simple yet effective Similarity Guidance
network to tackle the One-shot (SG-One) segmentation problem.
We aim at predicting the segmentation mask of a query image
with the reference to one densely labeled support image of the
same category. To obtain the robust representative feature of
the support image, we firstly adopt a masked average pooling
strategy for producing the guidance features by only taking the
pixels belonging to the support image into account. We then
leverage the cosine similarity to build the relationship between
the guidance features and features of pixels from the query image.
In this way, the possibilities embedded in the produced similarity
maps can be adapted to guide the process of segmenting objects.
Furthermore, our SG-One is a unified framework which can
efficiently process both support and query images within one
network and be learned in an end-to-end manner. We conduct
extensive experiments on Pascal VOC 2012. In particular, our One achieves the mIoU score of 46.3%, surpassing the baseline
methods.

Abstract— Few-shot image semantic segmentation tasks and methods
have been proposed and achieve better and better performance in recent years. However,
the purpose of these tasks need many samples as their seen class for mate learning.
In this paper, we propose a new and more challenging task
condition: weaklier One-shot segmentation with one image per category, 
which only provides prior knowledge that humans
need to recognize new objects, and aims to achieve pixel-level
object semantic understanding.  
To obtain the robust representative feature of the each category, we firstly
adopt a masked average pooling for producing the similarity matrix which show the
relationship between prototype and features of pixel from query image. Then we use
class token to remove the objects categories which not exist in query image. In 
this way, the semantic segmentation mask can be processed by similarity matrix.
Researches on PASCAL VOC 2012 dataset demonstrates
the effectiveness of the proposed method. In particular, our method achieves the mIoU score of 53.5%.


The main contributions of this paper are three-fold.
First, we propose a new more challenge and human-like learning
conditions for semantic segmentation, which is achieve pixel level
learning only use few samples.
Second, we can full use of pretrained model extracting the robust prototypes and producing
pixel-wise guidance using similarity matrix between 
prototypes and query features for predicting the segmentation masks
Third, experiments on PASCAL VOC 2012 dataset show
that the proposed method provides satisfactory performance
under fewer samples conditions. Our network achieves the 
mIoU of 53.5% on the PASCAL VOC 2012 dataset in our new
segmentation setting.



