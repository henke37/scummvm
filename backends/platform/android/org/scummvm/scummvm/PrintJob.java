package org.scummvm.scummvm;


import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.PixelFormat;
import android.graphics.Paint;

import android.print.PageRange;
import android.print.PrintAttributes;
import android.print.PrintDocumentAdapter;
import android.print.PrintDocumentAdapter;
import android.print.PrintDocumentInfo;
import android.print.PrintManager;
import android.print.pdf.PrintedPdfDocument;

import android.os.Bundle;
import android.os.CancellationSignal;
import android.os.ParcelFileDescriptor;

import android.util.Log;

public class PrintJob {

	private PrintDocAdapter adapter;
	
	private PrintedPdfDocument pdf;
	
	private Canvas pageCanvas;
	private Paint paint;
	
	public PrintJob(PrintManager manager, String title, PrintAttributes atts) {
		adapter=new PrintDocAdapter();
		
		Log.d(ScummVM.LOG_TAG, "new PrintJob");
		
		manager.print(title, adapter, atts);
	}

	private class PrintDocAdapter extends PrintDocumentAdapter {
	
		public void onStart () {
			Log.d(ScummVM.LOG_TAG, "PrintAdapter Start");
		}
		
		public void onLayout (PrintAttributes oldAttributes, 
                PrintAttributes newAttributes, 
                CancellationSignal cancellationSignal, 
                PrintDocumentAdapter.LayoutResultCallback callback, 
                Bundle extras) {
			Log.d(ScummVM.LOG_TAG, "PrintAdapter onLayout");
		}
		
		public void onWrite (PageRange[] pages, 
                ParcelFileDescriptor destination, 
                CancellationSignal cancellationSignal, 
                PrintDocumentAdapter.WriteResultCallback callback) {
			Log.d(ScummVM.LOG_TAG, "PrintAdapter onWrite");
		}
		
		public void onFinish () {
			Log.d(ScummVM.LOG_TAG, "PrintAdapter onFinish");
		}
	}
}